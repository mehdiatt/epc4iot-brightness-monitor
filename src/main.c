#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "led_rgb.h"

int main(void)
{
	static const enum led_color sequence[] = {
		LED_RED, LED_YELLOW, LED_GREEN, LED_BLUE, LED_OFF,
	};
	static const char *const names[] = {
		"RED", "YELLOW", "GREEN", "BLUE", "OFF",
	};

	if (led_rgb_init() < 0) {
		printk("Error: RGB LED init failed\n");
		return 0;
	}

	while (1) {
		for (int i = 0; i < ARRAY_SIZE(sequence); i++) {
			printk("LED: %s\n", names[i]);
			led_rgb_set(sequence[i]);
			k_sleep(K_SECONDS(1));
		}
	}
	return 0;
}
