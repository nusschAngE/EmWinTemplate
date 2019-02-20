
#ifndef _TIME_DATE_H
#define _TIME_DATE_H

#include "public.h"

#define COUNTS_PER_SECOND           (1000)

#define SECONDS_PER_MINUTE          (60)
#define MINUTES_PER_HOUR            (60)
#define HOURS_PER_DAY               (24)
#define DAYS_PER_YEAR               (365)
#define MONTHS_PER_YEAR             (12)

#define SECONDS_PER_HOUR            (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY             (HOURS_PER_DAY * SECONDS_PER_HOUR)
#define SECONDS_PER_YEAR            (DAYS_PER_YEAR * SECONDS_PER_DAY)


typedef struct
{
    uint16_t y;
    uint8_t  m;
    uint8_t  d;

    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
}TimeDate_t;

typedef struct
{
    uint8_t          set;
    
    TimeDate_t dt;
}SystemTime_t;

/* system time in seconds */
extern volatile uint16_t systemCount;
extern volatile uint64_t systemTime;



#endif

