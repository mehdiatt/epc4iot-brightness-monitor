#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "led_rgb.h"
#include "ldr.h"

int main(void)
{
	if (led_rgb_init() < 0) {
		printk("Error: RGB LED init failed\n");
		return 0;
	}
	if (ldr_init() < 0) {
		printk("Error: ADC / LDR init failed\n");
		return 0;
	}

	printk("Light sensor test: cover the sensor, then light it.\n");

	while (1) {
		int raw = ldr_read_raw();

		if (raw < 0) {
			printk("ADC read error: %d\n", raw);
		} else {
			printk("raw = %4d   brightness = %3d %%\n",
			       raw, ldr_raw_to_percent(raw));
		}
		k_sleep(K_MSEC(500));
	}
	return 0;
}
