
#ifndef _SDRAM_H
#define _SDRAM_H

#include "public.h"

enum
{
    SDRAM_ERROR_NONE = 0,
    SDRAM_ERROR_CMD,
    SDRAM_ERROR_INIT,
};

#define SDRAM_MEMSIZE              ((uint32_t)(32 * 1024 * 1024))//((uint32_t)0x2000000) //32MByte
/*  */
#define SDRAM_BASEADDRESS           ((uint32_t)0xc0000000)/* FMC Bank5 SDRAM */


extern uint8_t ExtSDRAM_Init(void);

#endif

