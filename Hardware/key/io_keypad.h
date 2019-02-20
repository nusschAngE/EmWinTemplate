
#ifndef _IO_KEYPAD_H
#define _IO_KEYPAD_H


#include "public.h"

/*  GPIO KEY
*/
enum
{
    GPIO_KEY0 = 0,
    GPIO_KEY1,
    GPIO_KEY2,
    GPIO_KEY3,

    GPIO_KEY_NUM,
};

#if 0
typedef struct
{
    uint8_t  (*Init)      (void);
    uint8_t  (*Scan)      (void);
    uint32_t (*GetToggle) (void);   
}IoKeyDrv_t;

extern IoKeyDrv_t IoKeyDrv;
#endif

extern uint8_t IoKey_Init(void);
extern uint32_t IoKey_GetToggle(void);
extern uint8_t IoKey_Scan(void);

#endif

