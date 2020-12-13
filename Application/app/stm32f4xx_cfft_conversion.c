#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include "arm_const_structs.h"

float32_t Input[512];
float32_t Output[512];

static const arm_cfft_instance_f32 *get_cfft_instanse(int fft_length)
{
    const arm_cfft_instance_f32 *S = NULL;
    
    switch (fft_length) {
    case 16:
      S = &arm_cfft_sR_f32_len16;
      break;
    case 32:
      S = &arm_cfft_sR_f32_len32;
      break;
    case 64:
      S = &arm_cfft_sR_f32_len64;
      break;
    case 128:
      S = &arm_cfft_sR_f32_len128;
      break;
    case 256:
      S = &arm_cfft_sR_f32_len256;
      break;
    case 512:
      S = &arm_cfft_sR_f32_len512;
      break;
    case 1024:
      S = &arm_cfft_sR_f32_len1024;
      break;
    case 2048:
      S = &arm_cfft_sR_f32_len2048;
      break;
    case 4096:
      S = &arm_cfft_sR_f32_len4096;
      break;
    }

    return S;
}

void stm32_cfft_convert(int16_t *buf, uint16_t fft_size)
{
    arm_q15_to_float(buf, Input, fft_size << 1);

    arm_cfft_f32(get_cfft_instanse(fft_size), Input, 0, 1);

    arm_cmplx_mag_f32(Input, Output, fft_size);

    buf[0] = 0;

    arm_float_to_q15(Output, buf, fft_size << 1);
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