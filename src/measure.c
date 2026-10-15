/*
 * Measure thread: samples the light sensor every 2 s, only while the
 * controller allows it (NORMAL mode).
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "events.h"
#include "ldr.h"

#define MEASURE_PERIOD_S  2

static void measure_thread(void *a, void *b, void *c)
{
	if (ldr_init() < 0) {
		printk("Error: ADC / LDR init failed\n");
		return;
	}

	while (1) {
		/* Sleeps here while the system is in BLUE or OFF mode. */
		k_event_wait(&meas_ev, MEAS_ENABLED, false, K_FOREVER);

		int raw = ldr_read_raw();

		if (raw >= 0) {
			struct app_event ev = {
				.type = EVT_BRIGHTNESS,
				.raw = raw,
				.percent = ldr_raw_to_percent(raw),
			};
			k_msgq_put(&event_q, &ev, K_NO_WAIT);
		} else {
			printk("ADC read error: %d\n", raw);
		}

		k_sleep(K_SECONDS(MEASURE_PERIOD_S));
	}
}

K_THREAD_DEFINE(measure_tid, 1024, measure_thread, NULL, NULL, NULL, 5, 0, 0);
