#ifndef __STM32_PWM_TIMER_INIT__
#define __STM32_PWM_TIMER_INIT__

void timer_pwm_init();

void TIM_start();
void TIM_stop();
void start_dma_wrapper(void *ptr, uint16_t size);
void stop_dma_wrapper();

#endif /* __STM32_PWM_TIMER_INIT__ */