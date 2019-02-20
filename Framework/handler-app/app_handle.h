
#ifndef _APP_HANDLE_H
#define _APP_HANDLE_H

#include "public.h"


/* app content */

typedef bool (*EvtHandler_t)(void *evt, void *p_arg);
typedef void (*AppUnload_t)(void);

typedef struct
{   
    EvtHandler_t EvtHandler;
    AppUnload_t  unload;
}AppContent_t;


/* app level */
typedef enum
{
    APP_LV_PLAY = 0,
    APP_LV_BK1  = 1,
    APP_LV_BK2  = 2,
    APP_LV_SYS  = 3,

    APP_LV_MAX,
}AppLevel_t;

extern AppContent_t *UiApp[APP_LV_MAX];

/****  *****/
extern void UiApp_Init(void);
extern void UiApp_Register(AppContent_t *app, AppLevel_t level);
extern void UiApp_Deregister(AppContent_t *app, AppLevel_t level);

#endif //_APP_HANDLE_H

