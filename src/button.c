#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "events.h"

#define LONG_PRESS_MS  1000

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static struct gpio_callback button_cb;

/* Raw edges from the ISR: 1 = pressed, 0 = released. */
K_MSGQ_DEFINE(edge_q, sizeof(uint8_t), 8, 1);

static void button_isr(const struct device *dev, struct gpio_callback *cb,
		       uint32_t pins)
{
	uint8_t pressed = (uint8_t)gpio_pin_get_dt(&button); /* 1 = pressed */

	k_msgq_put(&edge_q, &pressed, K_NO_WAIT);
}

static void send_event(enum app_event_type type)
{
	struct app_event ev = { .type = type };

	k_msgq_put(&event_q, &ev, K_NO_WAIT);
}

static int button_init(void)
{
	if (!gpio_is_ready_dt(&button)) {
		return -ENODEV;
	}
	if (gpio_pin_configure_dt(&button, GPIO_INPUT) < 0) {
		return -EIO;
	}
	gpio_init_callback(&button_cb, button_isr, BIT(button.pin));
	if (gpio_add_callback(button.port, &button_cb) < 0) {
		return -EIO;
	}
	return gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
}

static void button_thread(void *a, void *b, void *c)
{
	uint8_t level;

	if (button_init() < 0) {
		printk("Error: button B2 init failed\n");
		return;
	}

	while (1) {
		/* Wait for a press (ignore releases). */
		k_msgq_get(&edge_q, &level, K_FOREVER);
		if (!level) {
			continue;
		}

		int64_t t0 = k_uptime_get();
		bool released = false;

		/* Wait for the release, but no longer than LONG_PRESS_MS. */
		while (1) {
			int64_t left = LONG_PRESS_MS - (k_uptime_get() - t0);

			if (left <= 0 ||
			    k_msgq_get(&edge_q, &level, K_MSEC(left)) == -EAGAIN) {
				break;              /* timeout: long press */
			}
			if (!level) {
				released = true;    /* released before 1 s */
				break;
			}
			/* extra "pressed" edge (bounce): keep waiting */
		}

		if (released) {
			send_event(EVT_SHORT_PRESS);
		} else {
			/* Sent right when 1 s is reached, not on release. */
			send_event(EVT_LONG_PRESS);

			/* Wait for the release and swallow it, so a long press
			 * never produces a short press. */
			do {
				k_msgq_get(&edge_q, &level, K_FOREVER);
			} while (level);
		}
	}
}

K_THREAD_DEFINE(button_tid, 1024, button_thread, NULL, NULL, NULL, 3, 0, 0);
