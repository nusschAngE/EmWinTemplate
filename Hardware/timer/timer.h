
#ifndef _TIMER_H
#define _TIMER_H

#include "public.h"

typedef struct
{
    /*  TIM in 100MHZ
    */
    uint32_t arr;//auto reload value
    uint32_t psc;//prescaler
    
    void (*TriggerFunc)(void);
}TimInit_t;





/**** PUBLIC FUNCTION ****/
void TIM3_Init(TimInit_t *init);
void TIM3_DeInit(TimInit_t *init);

void TIM4_Init(TimInit_t *init);
void TIM4_DeInit(TimInit_t *init);


#endif //_TIM4_H

