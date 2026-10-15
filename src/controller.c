/*
 * Controller thread: finite state machine (NORMAL / BLUE / OFF).
 * It is the only thread that drives the RGB LED and prints the mode.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "events.h"
#include "led_rgb.h"

K_MSGQ_DEFINE(event_q, sizeof(struct app_event), 8, 4);
K_EVENT_DEFINE(meas_ev);

enum mode { MODE_NORMAL, MODE_BLUE, MODE_OFF };

static enum mode mode;

static enum led_color color_from_percent(int percent)
{
	if (percent < 33) {
		return LED_RED;
	}
	if (percent <= 66) {
		return LED_YELLOW;
	}
	return LED_GREEN;
}

static void enter_mode(enum mode m)
{
	mode = m;

	switch (m) {
	case MODE_NORMAL:
		led_rgb_set(LED_OFF);              /* until the first measure */
		k_event_set(&meas_ev, MEAS_ENABLED);
		printk("[MODE] NORMAL\n");
		break;
	case MODE_BLUE:
		k_event_set(&meas_ev, 0);
		led_rgb_set(LED_BLUE);
		printk("[MODE] BLUE\n");
		break;
	case MODE_OFF:
		k_event_set(&meas_ev, 0);
		led_rgb_set(LED_OFF);
		printk("System OFF\n");
		break;
	}
}

static void controller_thread(void *a, void *b, void *c)
{
	struct app_event ev;

	if (led_rgb_init() < 0) {
		printk("Error: RGB LED init failed\n");
		return;
	}
	printk("Brightness monitor ready (B2: short = switch mode, long = OFF)\n");
	enter_mode(MODE_NORMAL);

	while (1) {
		k_msgq_get(&event_q, &ev, K_FOREVER);

		switch (ev.type) {
		case EVT_SHORT_PRESS:
			/* NORMAL <-> BLUE ; OFF -> NORMAL */
			enter_mode(mode == MODE_NORMAL ? MODE_BLUE : MODE_NORMAL);
			break;

		case EVT_LONG_PRESS:
			if (mode != MODE_OFF) {
				enter_mode(MODE_OFF);
			}
			break;

		case EVT_BRIGHTNESS:
			/* Ignore a late measurement if we left NORMAL mode. */
			if (mode == MODE_NORMAL) {
				led_rgb_set(color_from_percent(ev.percent));
				printk("[NORMAL] ADC raw = %4d   brightness = %3d %%\n",
				       ev.raw, ev.percent);
			}
			break;
		}
	}
}

K_THREAD_DEFINE(controller_tid, 1024, controller_thread, NULL, NULL, NULL, 4, 0, 0);
