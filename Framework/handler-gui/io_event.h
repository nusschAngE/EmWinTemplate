
#ifndef _IO_EVENT_H
#define _IO_EVENT_H

#include "public.h"

/* IO Event */
enum
{
    IO_EVT_RIGHT,
    IO_EVT_DOWN,
    IO_EVT_LEFT,
    IO_EVT_UP,

    IO_EVT_MAX
};

/* System event */
enum
{
    SYS_EVT_NONE = 0,
    SYS_EVT_TICK_5MS,
    SYS_EVT_TICK_10MS,
    SYS_EVT_TICK_20MS,
    SYS_EVT_TICK_25MS,
    SYS_EVT_TICK_50MS,
    SYS_EVT_TICK_1S,

    SYS_EVT_MAX
};

/* key active type */
enum
{
    IO_KEY_NONE = 0,
    IO_KEY_DOWN,
    IO_KEY_UP_BEFORE_HOLD,
    IO_KEY_HOLD,
    IO_KEY_REPEAT,
    IO_KEY_UP_AFTER_HOLD,
};

/* tp active type */
enum
{
    TP_ACT_NONE = 0,
    TP_ACT_DOWN,
    TP_ACT_UP_BEFORE_HOLD,
    TP_ACT_HOLD,
    TP_ACT_REPEAT,
    TP_ACT_UP_AFTER_HOLD,    
};





#endif

