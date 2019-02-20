
#ifndef _DELAY_H
#define _DELAY_H

#include "public.h"




extern uint8_t DelayModule_Init(void);
extern void TimDelayUs(uint32_t us);
extern void TimDelayMs(uint32_t ms);
extern void SysTickDelay(uint32_t ticks);
extern void DelayUs(uint16_t us);
extern void DelayMs(uint16_t ms);
extern void OSDelay(uint32_t ticks);
extern bool WaitCondition(uint32_t ticks, bool(*condition)(void));
extern bool OSWaitCondition(uint32_t ticks, bool(*condition)(void));
#endif

