#include "stm32f4xx.h"
#include "stm324xg_eval.h"
#include "stm32f4_pwm_timer.h"

#include "audio_buffer.h"

#include "usbd_audio_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"

#include "adapter/adapter.h"

#include "console.h"

#include <stdbool.h>

extern void stm32_cfft_convert(int16_t *buf, uint16_t fft_size);
extern void stm32_normalise_function(int16_t *buf, uint16_t size);
bool Delay_non_blocking(uint32_t id, __IO uint32_t timeout);
void Delay_blocking(__IO uint32_t timeout);
void usb_sampling_wrapper(int16_t *samples, uint16_t size);

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

static __IO int32_t TimingDelay;

struct adapter *ws2812_adapter[CONFIG_IFNUM] = {NULL, NULL};

static struct source_config_function config0 =
{
  .base.type = SOURCE_TYPE_LINEAR,
  .k = 1,
  .b = 0,
  .y_max = 255,
  .change_step_b = 1,
  .change_step_k = 0
};

static struct source_config_function config1 =
{
  .base.type = SOURCE_TYPE_LINEAR,
  .k = 0,
  .b = 255,
  .y_max = 255,
  .change_step_b = 0,
  .change_step_k = 0
};

static struct source_config_function config2 =
{
  .base.type = SOURCE_TYPE_LINEAR,
  .k = 0,
  .b = 255,
  .y_max = 255,
  .change_step_b = 0,
  .change_step_k = 0
};

static struct source_config_music music = 
{
  .base.type = SOURCE_TYPE_MUSIC,
  .is_fft_conversion_async = false,
  .is_sampling_async = true,
  .sampling_fnc = usb_sampling_wrapper,
  .fft_convert_fnc = stm32_cfft_convert,
  .normalise_fnc = stm32_normalise_function
};

void USBAudioInit()
{
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
            USB_OTG_HS_CORE_ID,
#else
            USB_OTG_FS_CORE_ID, 
#endif
            &USR_desc, &AUDIO_cb, &USR_cb);
}

static struct ws2812_operation_fn_table fn = 
{
  .hw_start_dma = start_dma_wrapper,
  .hw_stop_dma = stop_dma_wrapper,
  .hw_start_timer = TIM_start,
  .hw_stop_timer = TIM_stop,
  .hw_delay = Delay_non_blocking
};

int main(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  struct source_config *first = (struct source_config *)(&config0);
  struct source_config *second = (struct source_config *)(&config1);
  struct source_config *third = (struct source_config *)(&config2);

  /* Initialize LEDS */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
       
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

  RCC_HSEConfig(RCC_HSE_ON);
  while(!RCC_WaitForHSEStartUp())
  {
  }

  timer_pwm_init();

  ws2812_adapter[0] = adapter_init(&fn, RGB, 118, 1);
  make_source_aggregator_from_config(&(ws2812_adapter[0]->aggregator), first, second, third);
  adapter_set_driver_id(ws2812_adapter[0], hw[0].id);
  adapter_start(ws2812_adapter[0]);

  config0.y_max = 360;
  config1.b = 0;
  config1.y_max = 100;
  config2.b = 0;
  config2.y_max = 100;

  ws2812_adapter[1] = adapter_init(&fn, HSV, 36, 100);
  make_source_aggregator_from_config(&(ws2812_adapter[1]->aggregator), first, second, third);
  adapter_set_driver_id(ws2812_adapter[1], hw[1].id);
  adapter_start(ws2812_adapter[1]);

  USBAudioInit();

  console_init();

  while (1)
  {
    adapter_process(ws2812_adapter, 2);
  }
}

void Delay_blocking(__IO uint32_t timeout)
{
  TimingDelay = (int32_t)(timeout & 0x7FFFFFFF);
  
  while(TimingDelay != 0);
}

bool Delay_non_blocking(uint32_t id, __IO uint32_t timeout)
{
  if(!hw[id].delay_is_waiting)
  {
    hw[id].delay_counter = (int32_t)(timeout & 0x7FFFFFFF);
    hw[id].delay_is_waiting = true;
  }

  if(hw[id].delay_counter <= 0)
  {
    hw[id].delay_is_waiting = false;
    return true;
  }

  return false;
}

void TimingDelay_Decrement(void)
{
  uint8_t i = 0;
  
  TimingDelay--;

  for(i = 0; i < IFNUM; i++)
  {
    hw[i].delay_counter--;
  }

}

void led_strip_dma_ISRHandler(int id)
{
  ws2812_adapter[id]->base.dma_interrupt(&(ws2812_adapter[id]->base));
}

void led_strip_timer_ISRHandler(int id)
{
  ws2812_adapter[id]->base.timer_interrupt(&(ws2812_adapter[id]->base));
}

void usb_samping_finish()
{
  sampling_async_finish(ws2812_adapter[0]->aggregator.first);
}

void usb_sampling_wrapper(int16_t *samples, uint16_t size)
{
  um_buffer_handle_register_listener(samples, size, usb_samping_finish);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

