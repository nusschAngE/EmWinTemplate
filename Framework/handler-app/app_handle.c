
#include "app_handle.h"
#include "ui_event.h"

/* App content */
AppContent_t *UiApp[APP_LV_MAX];

/******* PUBLIC FUNCTIONS *********/
void UiApp_Init(void)
{
    AppLevel_t lv;

    for(lv = APP_LV_PLAY; lv < APP_LV_MAX; lv++) {
        UiApp[lv] = NULL;
    }
}

void UiApp_Register(AppContent_t *app, AppLevel_t level)
{
    if(!app) {return ;}

    if((UiApp[level] != NULL)
        && (UiApp[level] != app))
    {
        if(UiApp[level]->unload != NULL) {
            UiApp[level]->unload();
        }
    }

    UiApp[level] = app;
}

void UiApp_Deregister(AppContent_t *app, AppLevel_t level)
{
    if((app != NULL)
        &&(UiApp[level] == app))
    {
        if(UiApp[level]->unload != NULL) {
            UiApp[level]->unload();
        }
        
        UiApp[level] = NULL;
    }
}

