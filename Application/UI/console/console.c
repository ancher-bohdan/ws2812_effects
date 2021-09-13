#include "stm32f4_usart_driver.h"

#include "adapter/adapter.h"

#include "stm32f4xx.h"
#define ARM_MATH_CM4
#include "arm_math.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define COM_BUFFER_SIZE   400

uint16_t rx_iterator = 0;
char rx_buffer[COM_BUFFER_SIZE];

char tx_buffer[COM_BUFFER_SIZE];

extern struct adapter *ws2812_adapter[CONFIG_IFNUM];

const char *shell = ">>>";

const char *generic_error = "Unknow command. Type help or ?\r\n";

const char *generic_command_error = "Wrong command usage. Type <command> help\r\n";

const char *help_msg =
"    === Welcome to LEDSTIP CONTROLLER ===   \r\n\
List of supported command:\r\n\
?;help -- print this message\r\n\
ifconfig -- print list of supported led interfaces on current board\r\n\
set -- configure any particular led interface\r\n\
\r\n\
For detaled description about command type <command> help\r\n";

const char *ifconfig_help_msg =
"ifconfig - prints information about LED interfaces and configurations\r\n"
"The format of the description of one string is the following:\r\n"
"led<if_id>: <if_status:up/down> <RGB/HSV> led_count=<led_count> delay_ms=<delay_ms> <--config0>\r\n";

float arm_sinus(float degree)
{
  float radians = (degree * PI) / 180;
  return arm_sin_f32(radians);
}

void set_led_count(int if_id, int ledcount)
{
  if(if_id >= 0 && if_id < CONFIG_IFNUM)
  {
    ws2812_adapter[if_id]->base.led_count = ledcount;
  }
}

void set_delay(int if_id, int delay)
{
  if(if_id >= 0 && if_id < CONFIG_IFNUM)
  {
    ws2812_adapter[if_id]->hw_delay = delay;
  }
}

struct source_config_trigonometric user_requested_config;

int set_config(int if_id, int conf_num, char *new_config)
{
  int ret = 0;
  int expected_ret = 5;
  int temp[expected_ret];

  if(!strncmp(new_config, "LIN", 3))
  {
    user_requested_config.base.base.type = SOURCE_TYPE_LINEAR;

    ret = sscanf(new_config, "LIN;%d;%d;%d;%d;%d", temp, temp + 1, temp + 2, temp + 3, temp + 4);

    if(ret == expected_ret)
    {
      user_requested_config.base.k = (uint8_t)temp[0];
      user_requested_config.base.b = (uint16_t)temp[1];
      user_requested_config.base.y_max = (uint16_t)temp[2];
      user_requested_config.base.change_step_k = (uint16_t)temp[3];
      user_requested_config.base.change_step_b = (uint16_t)temp[4];
    }
  }
  else if(!strncmp(new_config, "SIN", 3))
  {
    user_requested_config.base.base.type = SOURCE_TYPE_TRIGONOMETRIC;

    ret = sscanf(new_config, "SIN;%d;%d;%d;%d;%d", temp, temp + 1, temp + 2, temp + 3, temp + 4);

    if(ret == expected_ret)
    {
      user_requested_config.base.k = (uint8_t)temp[0];
      user_requested_config.base.b = (uint16_t)temp[1];
      user_requested_config.base.y_max = (uint16_t)temp[2];
      user_requested_config.base.change_step_k = (uint16_t)temp[3];
      user_requested_config.base.change_step_b = (uint16_t)temp[4];
    }

    user_requested_config.hw_sinus = arm_sinus;
  }
  else if(!strncmp(new_config, "MUS", 3))
  {
    expected_ret = 1;
  }
  else
  {
    return -1;
  }

  if(ret == expected_ret)
  {
    switch (conf_num)
    {
    case 0:
      make_source_aggregator_from_config(&(ws2812_adapter[if_id]->aggregator), (struct source_config *)&user_requested_config, NULL, NULL);
      break;
    case 1:
      make_source_aggregator_from_config(&(ws2812_adapter[if_id]->aggregator), NULL, (struct source_config *)&user_requested_config, NULL);
      break;
    case 2:
      make_source_aggregator_from_config(&(ws2812_adapter[if_id]->aggregator), NULL, NULL, (struct source_config *)&user_requested_config);
      break;
    default:
      return -1;
    }
  }
  else
  {
    return -1;
  }
  return 0;
}

void get_ledifs_description(char *dst)
{
  uint8_t i = 0;
  char *ptr = dst;
  
  uint8_t config_active_bank = 0;
  
  for(i = 0; i < CONFIG_IFNUM; i++)
  {    
    int ret = sprintf(ptr, "led%ld: ", ws2812_adapter[i]->base.id);  
    if(ret < 0) return;
    ptr += ret;

    if(ws2812_adapter[i]->is_continue)
    {
      strncpy(ptr, "up ", 3);
      ptr += 3;
    }
    else
    {
      strncpy(ptr, "down ", 5);
      ptr += 5;
    }

    if(ws2812_adapter[i]->convert_to_dma == __rgb2dma)
    {
      strncpy(ptr, "RGB ", 4);
      ptr += 4;
    }
    else if (ws2812_adapter[i]->convert_to_dma == __hsv2dma)
    {
      strncpy(ptr, "HSV ", 4);
      ptr += 4;
    }

    ret = sprintf(ptr, "led_count = %ld delay_ms = %ld\r\n", ws2812_adapter[i]->base.led_count, ws2812_adapter[i]->hw_delay);
    if(ret < 0) return;
    ptr += ret;

    ret = sprintf(ptr, "\t--config0=");
    ptr += ret;
    config_active_bank = AGGREGATOR_GET_ACTIVE_BANK(ws2812_adapter[i]->aggregator, 0);
    ret = get_source_description(ptr, ws2812_adapter[i]->aggregator.first[config_active_bank] );   
    if(ret < 0) return;
    ptr += ret;
    ret = sprintf(ptr, "\r\n");
    ptr += ret;

    ret = sprintf(ptr, "\t--config1=");
    ptr += ret;
    config_active_bank = AGGREGATOR_GET_ACTIVE_BANK(ws2812_adapter[i]->aggregator, 1);
    ret = get_source_description(ptr, ws2812_adapter[i]->aggregator.second[config_active_bank]);
    if(ret < 0) return;
    ptr += ret;
    ret = sprintf(ptr, "\r\n");
    ptr += ret;

    ret = sprintf(ptr, "\t--config2=");
    ptr += ret;
    config_active_bank = AGGREGATOR_GET_ACTIVE_BANK(ws2812_adapter[i]->aggregator, 0);
    ret = get_source_description(ptr, ws2812_adapter[i]->aggregator.third[config_active_bank]);
    if(ret < 0) return;
    ptr += ret;
    ret = sprintf(ptr, "\r\n");
    ptr += ret;
  }
}

void print_shell()
{
    uart_send_async(shell, strlen(shell), NULL);
}

static void command_parser()
{
    char *token = strtok(rx_buffer, " ");

    if(!strncmp(token, "?", 1) || !strncmp(token, "help", 4))
    {
        uart_send_async(help_msg, strlen(help_msg), print_shell);
    }
    else if (!strncmp(token, "ifconfig", 8))
    {
        token = strtok(NULL, " ");
        if(token == NULL)
        {
            get_ledifs_description(tx_buffer);
            uart_send_async(tx_buffer, strlen(tx_buffer), print_shell);
        }
        else if(!strncmp(token, "help", 4))
        {
            uart_send_async(ifconfig_help_msg, strlen(ifconfig_help_msg), print_shell);
        }
        else
        {
            uart_send_async(generic_command_error, strlen(generic_command_error), print_shell);
        }
    }
    else if (!strncmp(token, "set", 4))
    {
        int if_id = -1;
        int ret;
        token = strtok(NULL, " ");
        
        ret = sscanf(token, "led%d", &if_id);
        if(ret != 1) goto error;

        while((token = strtok(NULL, " ")) != NULL)
        {
            if(!strncmp(token, "led_count=", 10))
            {
                int led_count = 0;
                ret = sscanf(token, "led_count=%d", &led_count);
                if(ret != 1) goto error;
                set_led_count(if_id, led_count);
            }
            else if(!strncmp(token, "delay_ms=", 9))
            {
                int del = 0;
                ret = sscanf(token, "delay_ms=%d", &del);
                if(ret != 1) goto error;
                set_delay(if_id, del);
            }
            else if(!strncmp(token, "--conf", 6))
            {
                int conf_num = 0;
                ret = sscanf(token, "--conf%d", &conf_num);
                if((ret != 1) || (conf_num < 0) || (conf_num >= 3)) goto error;
                if(set_config(if_id, conf_num, token + 8) != 0) goto error;
            }
            else
            {
                goto error;
            }
        }
        uart_send_async("done\r\n", 7, print_shell);
        return;
error:
        uart_send_async("command syntax error\r\n", 23, print_shell);
        return;
    }
    else if(!strncmp(token, "\r", 2))
    {
        print_shell();
    }
    else
    {
        uart_send_async(generic_error, strlen(generic_error), print_shell);
    }
}

void console_init()
{
    memset(rx_buffer, 0, COM_BUFFER_SIZE);
    Console_ll_init();
}

void receive_char(char rx)
{
    if(rx == '\n')
    {
        return;
    }

    if(rx_iterator >= COM_BUFFER_SIZE)
    {
        //user input error; skip all meaningless char
        return;
    }

    rx_buffer[rx_iterator++] = rx;

    if(rx == '\r')
    {
        command_parser();
        memset(rx_buffer, 0, rx_iterator);
        rx_iterator = 0;
    }
}