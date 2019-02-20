
#ifndef _PRINTF_H
#define _PRINTF_H

#include "public.h"

extern uint8_t PrintfModule_Init(void);
extern void MemPrintf(const char *id, const void *src, uint16_t size);
extern int printf(const char *format, ...);

#endif


