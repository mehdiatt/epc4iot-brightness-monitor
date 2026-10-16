#ifndef LDR_H
#define LDR_H

/* Raw ADC range: 12 bits -> 0..4095 */
#define LDR_RAW_MAX 4095

#define LDR_RAW_DARK    2700
#define LDR_RAW_BRIGHT  4000

/* Configure the ADC channel. Returns 0 on success, negative on error. */
int ldr_init(void);

/* One ADC conversion. Returns the raw value (0..4095), or a negative error. */
int ldr_read_raw(void);

/* Convert a raw value to a percentage 0..100 over [LDR_RAW_DARK, LDR_RAW_BRIGHT]. */
int ldr_raw_to_percent(int raw);

#endif /* LDR_H */
