#ifndef __STM32_PWM_TIMER_INIT__
#define __STM32_PWM_TIMER_INIT__

#include <stdbool.h>

#define IFNUM   2

struct hw_lut
{
    uint32_t id;
    TIM_TypeDef *tim;
    TIM_TypeDef *tim_base;
    DMA_Stream_TypeDef *dma;
    uint32_t DMA_HTIF;
    uint32_t DMA_TCIF;
    volatile int32_t delay_counter;
    bool delay_is_waiting;
};

extern struct hw_lut hw[IFNUM];

void timer_pwm_init();

void TIM_start(uint32_t driver_id);
void TIM_stop(uint32_t driver_id);
void start_dma_wrapper(uint32_t driver_id, void *ptr, uint16_t size);
void stop_dma_wrapper(uint32_t driver_id);

void TIM13_start();
void TIM13_stop();
void start_dma3_wrapper(void *ptr, uint16_t size);
void stop_dma3_wrapper();

#endif /* __STM32_PWM_TIMER_INIT__ */