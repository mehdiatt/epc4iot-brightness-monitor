#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "led_rgb.h"

/* ACTIVE_LOW is declared in the overlay, so "1" here always means "lit". */
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(rgb_red), gpios);
static const struct gpio_dt_spec led_g = GPIO_DT_SPEC_GET(DT_ALIAS(rgb_green), gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(rgb_blue), gpios);

int led_rgb_init(void)
{
	const struct gpio_dt_spec *leds[] = { &led_r, &led_g, &led_b };

	for (int i = 0; i < 3; i++) {
		if (!gpio_is_ready_dt(leds[i])) {
			return -ENODEV;
		}
		int ret = gpio_pin_configure_dt(leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			return ret;
		}
	}
	return 0;
}

void led_rgb_set(enum led_color color)
{
	gpio_pin_set_dt(&led_r, color == LED_RED   || color == LED_YELLOW);
	gpio_pin_set_dt(&led_g, color == LED_GREEN || color == LED_YELLOW);
	gpio_pin_set_dt(&led_b, color == LED_BLUE);
}
