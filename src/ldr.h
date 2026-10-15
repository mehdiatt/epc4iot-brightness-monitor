#ifndef LDR_H
#define LDR_H

/* Raw ADC range: 12 bits -> 0..4095 */
#define LDR_RAW_MAX 4095

/* Configure the ADC channel. Returns 0 on success, negative on error. */
int ldr_init(void);

/* One ADC conversion. Returns the raw value (0..4095), or a negative error. */
int ldr_read_raw(void);

/* Convert a raw value to a percentage 0..100. */
int ldr_raw_to_percent(int raw);

#endif /* LDR_H */
