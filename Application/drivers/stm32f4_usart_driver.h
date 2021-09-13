#ifndef __USART_DRIVER_INIT__
#define __USART_DRIVER_INIT__

void Console_ll_init();
void uart_send_async(const char *buf, int size, void (*completeon_fnc)());

#endif