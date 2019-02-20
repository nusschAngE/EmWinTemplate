
#ifndef _SYS_I2C_H
#define _SYS_I2C_H

#include "public.h"


enum
{
    IIC_OK = 0,
    IIC_TIMEOUT,
    IIC_ERROR,
};

/*  public
*/
void SYSI2C_Init(void);
void SYSI2C_Start(void);
void SYSI2C_SendByte(uint8_t sVal);
uint8_t SYSI2C_WaitACK(void);
uint8_t SYSI2C_ReadByte(void);
void SYSI2C_SendACK(void);
void SYSI2C_SendNACK(void);
void SYSI2C_Stop(void);



#endif

