#include "stm32f4xx.h"

    /**TIM2 GPIO Configuration   
    PA0-WKUP     ------> TIM2_CH1
    PD13         ------> TIM4_CH2
    */

#include "stm32f4_pwm_timer.h"

struct hw_lut hw[IFNUM] = 
{       //id    tim     tim_base        dma_stream      DMA_HT_IT_MASK          DMA_TC_IT_MASK          delay_counter    delay_is_waiting
        {0,     TIM2,   TIM14,          DMA1_Stream5,   DMA_IT_HTIF5,           DMA_IT_TCIF5,           0,               false},
        {1,     TIM4,   TIM13,          DMA1_Stream3,   DMA_IT_HTIF3,           DMA_IT_TCIF3,           0,               false}
};

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

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
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

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

        TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

        TIM_OC2Init(TIM4, &TIM_OCInitStructure);

        TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

        TIM_ARRPreloadConfig(TIM4, ENABLE);
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
#ifdef CONFIG_BUFFER_COUNT_4
        DMA_DoubleBufferModeCmd(DMA1_Stream5, ENABLE);
#endif

        TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        DMA_DeInit(DMA1_Stream3);
        DMA_InitStructure.DMA_Channel = DMA_Channel_2;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(TIM4->CCR2));
        DMA_Init(DMA1_Stream3, &DMA_InitStructure);
#ifdef CONFIG_BUFFER_COUNT_4
        DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);
#endif
        TIM_DMACmd(TIM4, TIM_DMA_CC2, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
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

        TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);

        TIM_PrescalerConfig(TIM14, PrescalerValue, TIM_PSCReloadMode_Immediate);

        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 40961;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

        TIM_OC1Init(TIM14, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Disable);

        TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
        
        NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;
        NVIC_Init(&NVIC_InitStructure);

        TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);

        TIM_PrescalerConfig(TIM13, PrescalerValue, TIM_PSCReloadMode_Immediate);

        TIM_OC1Init(TIM13, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Disable);

        TIM_ITConfig(TIM13, TIM_IT_CC1, ENABLE);
}

void timer_pwm_init()
{
        gpio_init();

        tim_init();

        dma_init();

        tim_time_base_init();
}

void TIM_start(uint32_t driver_id)
{
        TIM_Cmd(hw[driver_id].tim_base, ENABLE);
}

void TIM_stop(uint32_t driver_id)
{
        TIM_Cmd(hw[driver_id].tim_base, DISABLE);
}

void start_dma_wrapper(uint32_t driver_id, void *ptr, uint16_t size)
{
        hw[driver_id].dma->M0AR = (uint32_t)ptr;
#ifdef CONFIG_BUFFER_COUNT_4
        hw[driver_id].dma->NDTR = (uint32_t)size >> 1;
        DMA_DoubleBufferModeConfig(hw[driver_id].dma, (uint32_t)(((uint32_t *)ptr) + (size >> 1)), DMA_Memory_0);
#elif CONFIG_BUFFER_COUNT_2
        hw[driver_id].dma->NDTR = (uint32_t)size;
#endif
        DMA_ClearITPendingBit(hw[driver_id].dma, hw[driver_id].DMA_HTIF | hw[driver_id].DMA_TCIF);
        DMA_ITConfig(hw[driver_id].dma, DMA_IT_TC | DMA_IT_HT, ENABLE);
        DMA_Cmd(hw[driver_id].dma, ENABLE);
        TIM_Cmd(hw[driver_id].tim, ENABLE);
}

void stop_dma_wrapper(uint32_t driver_id)
{
        DMA_ITConfig(hw[driver_id].dma, DMA_IT_TC | DMA_IT_HT, DISABLE);
        DMA_Cmd(hw[driver_id].dma, DISABLE);
        TIM_Cmd(hw[driver_id].tim, DISABLE);
}

void TIM13_start()
{
        TIM_Cmd(TIM13, ENABLE);
}

void TIM13_stop()
{
        TIM_Cmd(TIM13, DISABLE);
}

void start_dma3_wrapper(void *ptr, uint16_t size)
{
        DMA1_Stream3->M0AR = (uint32_t)ptr;
        DMA1_Stream3->NDTR = (uint32_t)size;
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_HTIF3 | DMA_IT_TCIF3);
        DMA_ITConfig(DMA1_Stream3, DMA_IT_TC | DMA_IT_HT, ENABLE);
        DMA_Cmd(DMA1_Stream3, ENABLE);
        TIM_Cmd(TIM4, ENABLE);
}

void stop_dma3_wrapper()
{
        DMA_ITConfig(DMA1_Stream3, DMA_IT_TC | DMA_IT_HT, DISABLE);
        DMA_Cmd(DMA1_Stream3, DISABLE);
        TIM_Cmd(TIM4, DISABLE);
}