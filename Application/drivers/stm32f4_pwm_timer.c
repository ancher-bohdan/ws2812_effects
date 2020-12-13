#include "stm32f4xx.h"

    /**TIM2 GPIO Configuration   
    PA0-WKUP     ------> TIM2_CH1
    */

static void gpio_init()
{
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
}

static void tim_init()
{
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
        TIM_OCInitTypeDef  TIM_OCInitStructure;

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

        /* Time base configuration */
        TIM_TimeBaseStructure.TIM_Period = 104;
        TIM_TimeBaseStructure.TIM_Prescaler = 0;
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        /* PWM1 Mode configuration: Channel1 */
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 0;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

        TIM_OC1Init(TIM2, &TIM_OCInitStructure);

        TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

        TIM_ARRPreloadConfig(TIM2, ENABLE);
}

static void dma_init()
{
        DMA_InitTypeDef DMA_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
        
        DMA_DeInit(DMA1_Stream5);
        DMA_InitStructure.DMA_Channel = DMA_Channel_3;  
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(TIM2->CCR1));
        DMA_InitStructure.DMA_BufferSize = 1;
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_High;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

        DMA_Init(DMA1_Stream5, &DMA_InitStructure);

        TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
}

static void tim_time_base_init()
{
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
        TIM_OCInitTypeDef  TIM_OCInitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 6000000) - 1;

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = TIM8_TRG_COM_TIM14_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_TimeBaseStructure.TIM_Period = 65535;
        TIM_TimeBaseStructure.TIM_Prescaler = 0;
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

        TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

        TIM_PrescalerConfig(TIM14, PrescalerValue, TIM_PSCReloadMode_Immediate);

        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 40961;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

        TIM_OC1Init(TIM14, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Disable);

        TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);

        TIM_Cmd(TIM14, ENABLE);
}

void timer_pwm_init()
{
        gpio_init();

        tim_init();

        dma_init();

        tim_time_base_init();
}

void TIM_start()
{
        TIM_Cmd(TIM14, ENABLE);
}

void TIM_stop()
{
        TIM_Cmd(TIM14, DISABLE);
}

void start_dma_wrapper(void *ptr, uint16_t size)
{
        DMA1_Stream5->M0AR = (uint32_t)ptr;
        DMA1_Stream5->NDTR = (uint32_t)size;
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5 | DMA_IT_TCIF5);
        DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);
        DMA_Cmd(DMA1_Stream5, ENABLE);
        TIM_Cmd(TIM2, ENABLE);
}

void stop_dma_wrapper()
{
        DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, DISABLE);
        DMA_Cmd(DMA1_Stream5, DISABLE);
        TIM_Cmd(TIM2, DISABLE);
}