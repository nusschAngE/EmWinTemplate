
#include <stdio.h>
#include "stm32h7xx.h"
#include "uart_debug.h"


/* USART Handler */
static UART_HandleTypeDef DBGUart_Handle;

/***************** private ********************/
/*
*   
*/
#if 0
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_Init;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
	
    GPIO_Init.Pin = GPIO_PIN_9 | GPIO_PIN_10;		
    GPIO_Init.Mode = GPIO_MODE_AF_PP;		
    GPIO_Init.Pull = GPIO_PULLUP;			
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Alternate = GPIO_AF7_USART1;	
    HAL_GPIO_Init(GPIOA, &GPIO_Init);	   		  
}
#endif

uint8_t DebugUart_Init(uint32_t BaudRate)
{
    GPIO_InitTypeDef GPIO_Init;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
	/* GPIO Init */
    GPIO_Init.Pin = GPIO_PIN_9 | GPIO_PIN_10;		
    GPIO_Init.Mode = GPIO_MODE_AF_PP;		
    GPIO_Init.Pull = GPIO_PULLUP;			
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Alternate = GPIO_AF7_USART1;	
    HAL_GPIO_Init(GPIOA, &GPIO_Init);	   		 
    /* UART Init */
	DBGUart_Handle.Instance = USART1;					
	DBGUart_Handle.Init.BaudRate = BaudRate;				  
	DBGUart_Handle.Init.WordLength = UART_WORDLENGTH_8B;  
	DBGUart_Handle.Init.StopBits = UART_STOPBITS_1;	   
	DBGUart_Handle.Init.Parity = UART_PARITY_NONE;		  
	DBGUart_Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;   
	DBGUart_Handle.Init.Mode = UART_MODE_TX;//UART_MODE_TX_RX	
	DBGUart_Handle.Init.Prescaler = UART_PRESCALER_DIV12;//20mhz
	DBGUart_Handle.Init.OverSampling = UART_OVERSAMPLING_16;
	DBGUart_Handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	DBGUart_Handle.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	if(HAL_UART_Init(&DBGUart_Handle) == HAL_OK)
	{
	    return (0);//ERROR NONE
	}
	else
	{
	    return (1);//ERROR
	}
}

void DebugUart_SendChar(uint8_t ch)
{
    //__HAL_UART_CLEAR_FLAG(&DBGUart_Handler, UART_FLAG_TC);

    while(__HAL_UART_GET_FLAG(&DBGUart_Handle, UART_FLAG_TXE) == RESET)
    {
    }
    __HAL_UART_CLEAR_FLAG(&DBGUart_Handle, UART_FLAG_TXE);
    DBGUart_Handle.Instance->TDR = ch;      
}

