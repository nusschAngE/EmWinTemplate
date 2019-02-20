
#ifndef _UART_ATK_H
#define _UART_ATK_H

#include "public.h"

typedef void (*ATK_RxParserCallback)(uint8_t data);
typedef void (*ATK_RxCpltCallback)(void);
typedef void (*ATK_TxCpltCallback)(void);
typedef void (*ATK_ErrorCallback)(void);

typedef struct
{
    uint32_t baudrate;
    uint32_t wordlength;
    uint32_t stopbits;
    uint32_t parity;
    uint32_t mode;
    uint32_t hwflowctrl;
    ATK_RxParserCallback rxParserFunc;
    ATK_RxCpltCallback rxCpltProcFunc;
    ATK_TxCpltCallback txCpltProcFunc;
    ATK_ErrorCallback errorProcFunc;
}UartDevice_t;

/* PUBLIC FUNCTION */
uint8_t ATKUart_Init(UartDevice_t *dev);
void ATKUart_DeInit(UartDevice_t *dev);
void ATKUart_TransmitIT(uint8_t *pData, uint16_t size);
void ATKUart_ReceiveIT(uint8_t *pBuff, uint16_t size);

#endif

