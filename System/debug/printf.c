
#include <stdio.h>
#include "uart_debug.h"



/*********** usart debug configuration *************/

uint8_t PrintfModule_Init(void)
{
    return DebugUart_Init(PRINTF_BAUDRATE);
}

void MemPrintf(const char *id, const void *src, uint16_t size)
{
    uint16_t i = 0;
    const uint8_t *ptr = (const uint8_t *)src;

    printf("%s, memory = %p, size = %d\r\n", id, src, size);
    for(i = 0; i < size; i++)
    {
        if((i) && (i % 32 == 0)) printf("\r\n");

        printf("%02X ", ptr[i]);
    }
    printf("\r\n");
}

/********** printf config ***********/
//#pragma import(__use_no_semihosting)             
              
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
  
void _sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{ 	
	DebugUart_SendChar((uint8_t)ch);
	
	return ch;
}


