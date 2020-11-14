#ifndef __ADC_DRIVER__
#define __ADC_DRIVER__

void adc_init(void);

void adc_on(void);
void adc_start(uint16_t *samples_buffer, uint32_t samples_number);

#endif
