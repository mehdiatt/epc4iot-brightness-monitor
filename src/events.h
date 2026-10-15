#ifndef EVENTS_H
#define EVENTS_H

#include <zephyr/kernel.h>

/* Messages sent to the controller thread. */
enum app_event_type {
	EVT_SHORT_PRESS,   /* B2 short press                     */
	EVT_LONG_PRESS,    /* B2 held >= 1 s (sent immediately)  */
	EVT_BRIGHTNESS,    /* new ADC measurement                */
};

struct app_event {
	enum app_event_type type;
	int raw;       /* ADC raw value (EVT_BRIGHTNESS only)   */
	int percent;   /* brightness 0..100 (EVT_BRIGHTNESS)    */
};

/* Bit set while the measurement thread is allowed to sample the ADC. */
#define MEAS_ENABLED  BIT(0)

/* Defined in controller.c, used by the button and measure threads. */
extern struct k_msgq event_q;
extern struct k_event meas_ev;

#endif /* EVENTS_H */
