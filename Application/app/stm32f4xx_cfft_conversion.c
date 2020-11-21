#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include "arm_math.h"

float32_t Input[256];
float32_t Output[128];

void stm32_cfft_convert(int16_t *buf, uint16_t fft_size)
{
    int i = 0;
    arm_cfft_radix4_instance_f32 cfft;

    arm_cfft_radix4_init_f32(&cfft, fft_size, 0, 1);

    for(i = 0; i < (fft_size << 1); i++)
    {
        Input[i] = (float32_t)buf[i];
    }

    arm_cfft_radix4_f32(&cfft, Input);

    arm_cmplx_mag_f32(Input, Output, fft_size);

    buf[0] = 0;

    for(i = 1; i < (fft_size << 1); i++)
    {
        buf[i] = (int16_t)Output[i];
    }
}

void stm32_normalise_function(int16_t *buf, uint16_t size)
{
    uint32_t dummy;
    q15_t max_value;
    float32_t k = 0;
    int i = 0;

    arm_max_q15(buf, size, &max_value, &dummy);

    k = 360.0 / max_value;

    for(i = 0; i < size; i++)
    {
        buf[i] = (int16_t)(buf[i] * k);
    }
}