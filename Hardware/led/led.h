
#ifndef _LED_H
#define _LED_H

#include "public.h"

#define LED_NONE    0
#define LED_GREEN   1
#define LED_RED     2
#define LED_ALL     3

#if 0
typedef struct
{
    uint8_t (*Init)   (void);
    void    (*Toggle) (uint8_t led);
    void    (*Onoff)  (uint8_t led, bool on);
}LedDrv_t;

extern LedDrv_t LedDrv;
#endif

extern uint8_t LED_Init(void);
extern void LED_Toggle(uint8_t led);
extern void LED_Onoff(uint8_t led, bool on);

#endif

