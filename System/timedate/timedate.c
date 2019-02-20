
#include "stm32h7xx.h"

#include "timedate.h"

#if 0
#define LEAP_YEAR(year) ((!(year % 4) && (year % 100)) || !(year % 400))

static const uint8_t DaysInMonth[] = 
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
#endif

/* system time in seconds */
volatile uint16_t systemCount = 0;
volatile uint64_t systemTime = 0;


uint8_t TimeDateCaculate(TimeDate_t *dt)
{
    return (0);
}

/* 0 : invalide, 1~7 : week */
uint8_t WeekCaculate(TimeDate_t dt)
{
    return (0);
}






