#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include "ldr.h"

/* ADC channel described in the overlay (zephyr,user -> io-channels). */
static const struct adc_dt_spec adc_ch =
	ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static int16_t sample_buf;
static struct adc_sequence sequence = {
	.buffer = &sample_buf,
	.buffer_size = sizeof(sample_buf),
};

int ldr_init(void)
{
	int ret;

	if (!adc_is_ready_dt(&adc_ch)) {
		return -ENODEV;
	}
	ret = adc_channel_setup_dt(&adc_ch);
	if (ret < 0) {
		return ret;
	}
	return adc_sequence_init_dt(&adc_ch, &sequence);
}

int ldr_read_raw(void)
{
	int ret = adc_read_dt(&adc_ch, &sequence);

	if (ret < 0) {
		return ret;
	}
	return sample_buf;
}

int ldr_raw_to_percent(int raw)
{
	if (raw < 0) {
		raw = 0;
	}
	if (raw > LDR_RAW_MAX) {
		raw = LDR_RAW_MAX;
	}
	return (raw * 100) / LDR_RAW_MAX;
}
