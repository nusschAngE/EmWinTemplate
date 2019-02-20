
#ifndef _UART_DEBUG_H
#define _UART_DEBUG_H

#include "public.h"



#define PRINTF_BAUDRATE         (115200)

extern uint8_t DebugUart_Init(uint32_t BaudRate);
extern void DebugUart_SendChar(uint8_t ch);


#endif

