
#ifndef _IO_EXPAND_H
#define _IO_EXPAND_H

#include "public.h"

/* IO */
#define IOEX_BEEP_PWR         (0x01)
#define IOEX_AP32xx_INT       (0x02)
#define IOEX_DCMI_PWR         (0x04)
#define IOEX_USB_PWR          (0x08)
#define IOEX_USER_EX          (0x10)
#define IOEX_MPU_INT          (0x20)
#define IOEX_RX485_RE         (0x40)
#define IOEX_ETH_REST         (0x80)

#define IOEX_ALL_PIN          (0xFF)

/* public */
uint8_t IOEx_Init(void);
void IOEx_SetPin(uint8_t Pin, uint8_t sta);
uint8_t IOEx_GetPin(uint8_t Pin);
void IOEx_IntCheck(void);

#endif

