#ifndef LED_RGB_H
#define LED_RGB_H

enum led_color {
	LED_OFF,
	LED_RED,
	LED_YELLOW,   /* red + green */
	LED_GREEN,
	LED_BLUE,
};

/* Configure the 3 GPIOs. Returns 0 on success, negative on error. */
int led_rgb_init(void);

/* Show one colour (all other channels are switched off). */
void led_rgb_set(enum led_color color);

#endif /* LED_RGB_H */
