
#ifndef _TRACK_KPD_H
#define _TRACK_KPD_H

#include "public.h"


/* key map */
typedef struct
{
    uint8_t kpd;
    uint8_t key;
    uint8_t holdDelay;
    uint8_t repeatDelay;
}IOKeyMap_t;

typedef struct
{
    uint8_t code;
    uint8_t key;
    uint8_t holdDelay;
    uint8_t repeatDelay;
}IRKeyMap_t;

#define KEY_TRACK_TICK_PERIOD_MS    (10)//10MS
/* key track ticks */
#define KEY_DOWN_DEFAULT_DELAY      (5)
#define KEY_HOLD_DEFAULT_DELAY      (100)
#define KEY_REPEAT_DEFAULT_DELAY    (20)


extern uint8_t KPD_TrackInit(void);
extern void KPD_TrackTask(void *p_arg);


#endif


