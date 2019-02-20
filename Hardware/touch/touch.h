
#ifndef _TOUCH_H
#define _TOUCH_H

#include "public.h"

#define TOUCH_DRIVER_IC_GT9147  1U

/*  TP SCAN TYPE
*/
#define TP_SCAN_POLLING     0
#define TP_SCAN_TRIGGER     1

/************** TP DEFINES ******************/
#define TP_MAX_POINT    (5)

#define TP_POINT1       (0x01)
#define TP_POINT2       (0x02)
#define TP_POINT3       (0x04)
#define TP_POINT4       (0x08)
#define TP_POINT5       (0x10)

/* max size */
#define TOUCH_HW_W      (480)
#define TOUCH_HW_H      (800)

typedef struct
{
    uint8_t actPoints;
    
    int xPos[TP_MAX_POINT];
    int yPos[TP_MAX_POINT];    
}TPPoints_t;

/* TP DEVICE */
extern TPPoints_t tpPoints;

extern uint8_t TP_Init(void);
extern bool TP_ReadInterrupt(void);
extern uint8_t TP_PointChecked(void);
extern uint8_t TP_ReadPoint(uint8_t point, uint16_t *xPos, uint16_t *yPos);
extern void TP_ReadPoints(TPPoints_t *points);
extern void TP_Release(TPPoints_t *points);
#endif

