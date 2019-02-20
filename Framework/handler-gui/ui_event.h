
#ifndef _EVENT_H
#define _EVENT_H

#include "public.h"
#include "io_event.h"

/* mask */
#define UI_EVT_GROUP_MASK           ((uint32_t)0xFF000000)
#define UI_EVT_INDEX_MASK           ((uint32_t)0x00FFFF00)
#define UI_EVT_FLAG_MASK            ((uint32_t)0x000000FF)

#define TP_EVT_FLAG_MASK            ((uint32_t)0x000000FF)
#define TP_EVT_GESTRUE_MASK         ((uint32_t)0x0000FF00)

/* event group */
#define UI_EVT_GROUP_SYS            ((uint32_t)0x01000000)
#define UI_EVT_GROUP_IO             ((uint32_t)0x02000000)
#define UI_EVT_GROUP_TP             ((uint32_t)0x03000000)


/* get event */
#define UI_EVT_SYS(evt)             (UI_EVT_GROUP_SYS + ((uint32_t)evt << 8))
#define UI_EVT_IO(evt, flag)        (UI_EVT_GROUP_IO + ((uint32_t)evt << 8) + (uint32_t)flag)
#define UI_EVT_TP_PRESS(flag)       (UI_EVT_GROUP_TP + (uint32_t)flag)
#define UI_EVT_TP_GESTURE(gust)     (UI_EVT_GROUP_TP + ((uint32_t)gust << 8))


/* ui event struct */
typedef struct
{
    uint32_t code;
    int32_t  data;
}UiEvent_t;

/* io event */
typedef struct
{
    int32_t  data;
    uint16_t code;
    uint8_t  flag;    
}IoEvent_t;

/* tp event */
typedef struct
{
    uint16_t xPos;
    uint16_t yPos;
    
    uint8_t flag;
    uint8_t point;
    uint8_t gesture;
}TpEvent_t;

/* system event */
typedef struct
{
    uint32_t code;
    int32_t  data;
}SysEvent_t;

/* UI Message */
#define MAX_UI_MESG     (24u)

typedef struct
{
    bool    empty;
    
    UiEvent_t evt;    
}UIMessage_t;

extern uint8_t UiEvtInit(void);
extern uint8_t UiSendIoEvt(IoEvent_t *evt);
extern uint8_t UiSendTpEvt(TpEvent_t *evt);
extern uint8_t UiSendSysEvt(SysEvent_t *evt);
extern uint8_t UiGetUiEvt(UiEvent_t *evt);
extern void UiEvtGetXY(UiEvent_t *evt, uint16_t *xPos, uint16_t *yPos);
extern uint32_t UiEvtGetCode(UiEvent_t *evt);
extern uint8_t UiEvtGetFlag(UiEvent_t *evt);
extern int32_t UiEvtGetData(UiEvent_t *uiEvt);
extern void UiEvtFromIO(UiEvent_t *uiEvt, IoEvent_t *ioEvt);
extern void UiEvtFromTP(UiEvent_t *uiEvt, TpEvent_t *tpEvt);
extern void UiEvtFromSYS(UiEvent_t *uiEvt, SysEvent_t *sysEvt);
extern void TpEvtFromUI(UiEvent_t *uiEvt, TpEvent_t *tpEvt);






#endif

