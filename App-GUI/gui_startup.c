
#include "stm32h7xx.h"
#include "ucos_ii.h"
#include "fatfs_ex.h"
#include "led.h"

#include "track_kpd.h"
#include "track_touch.h"
#include "track_usbh.h"

#include "GUI.h"
#include "WM.h"
#include "ui_event.h"
#include "app_handle.h"

#include "app_home.h"
#include "gui_startup.h"
#include "app_esp8266_template.h"

#define APP_HANDLE_TICK     (20U)

/* task stack */

__ALIGNED(4) OS_STK StartupTaskStk[STARTUP_TASK_STK_SIZE];

__ALIGNED(4) OS_STK AppTaskStk[APP_TASK_STK_SIZE];
__ALIGNED(4) OS_STK KpdTaskStk[KPD_TASK_STK_SIZE];
__ALIGNED(4) OS_STK TpTaskStk[TP_TASK_STK_SIZE];

__ALIGNED(4) OS_STK LedTaskStk[LED_TASK_STK_SIZE];

/* test led task */

static void LED_Task(void *p_arg)
{
    (void)p_arg;

    while(1)
    {
        LED_Toggle(LED_RED);
        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}

static void SystemLoad(void *p_arg)
{
    /* emWin Init */
    GUI_Init();
    WM_SetCreateFlags(WM_CF_MEMDEV);
    WM_MULTIBUF_Enable(1);
		/* must load the first app */
    AppMenu_Load(p_arg);
}

static void UiApp_Handler(void *p_arg)
{
    UiEvent_t uiEvt;
    AppLevel_t appLevel = APP_LV_PLAY;
    bool handled = FALSE;

		SystemLoad(NULL);
    /* app handle loop */
    while(1)
    {
        if(UiGetUiEvt(&uiEvt) != 0) {//get ui event error 
            uiEvt.code = uiEvt.data = 0;
        }
    
        for(appLevel = APP_LV_PLAY; appLevel < APP_LV_MAX; appLevel++)
        {        
            if(UiApp[appLevel] != NULL) 
            {
                if(UiApp[appLevel]->EvtHandler != NULL) 
                {
                    handled = UiApp[appLevel]->EvtHandler(&uiEvt, NULL);
                    if(handled) {//clear the uiEvt 
                        uiEvt.code = uiEvt.data = 0;
                    }
                }
            }
        }
        
        /* delete the uiEvt*/
        uiEvt.code = uiEvt.data = 0;
        /* Delay */
        GUI_Delay(APP_HANDLE_TICK);
    }
}

static void EmWin_Startup(void)
{
    
    //GUI_CURSOR_Hide();
}

/************ PUBLIC FUNCTION *****************/

void GUI_Startup(void)
{
    //uint8_t ret = 0;
#if OS_CRITICAL_METHOD == 3u    
    OS_CPU_SR cpu_sr;
#endif

    /* uCOS Init */
    OSInit();
    /*  */
    OS_ENTER_CRITICAL();

    /* IO Init */
    UiEvtInit();
    KPD_TrackInit();

    /* emWin GUI Startup */
    EmWin_Startup();
    
#if 1
	/* app handler task */
	OSTaskCreateExt(
	                UiApp_Handler,
	                //EspTemp_Load,
                    //Emwin_DemoTask,
					NULL, 
					&AppTaskStk[APP_TASK_STK_SIZE - 1u], 
					APP_TASK_PRIO, 
					APP_TASK_PRIO, 
					&AppTaskStk[0u], 
					APP_TASK_STK_SIZE, 
					NULL, 
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif 
#if 1
	/* key track task */
	OSTaskCreateExt(KPD_TrackTask, 
					NULL, 
					&KpdTaskStk[KPD_TASK_STK_SIZE - 1u], 
					KPD_TASK_PRIO, 
					KPD_TASK_PRIO, 
					&KpdTaskStk[0u], 
					KPD_TASK_STK_SIZE, 
					NULL, 
					OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#if 1
    /* tp track task */
    OSTaskCreateExt(TP_TrackTask, 
                    NULL, 
                    &TpTaskStk[TP_TASK_STK_SIZE - 1u], 
                    TP_TASK_PRIO, 
                    TP_TASK_PRIO, 
                    &TpTaskStk[0u], 
                    TP_TASK_STK_SIZE, 
                    NULL, 
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
#if 1   
    /* led test task */
    OSTaskCreateExt(LED_Task, 
                    NULL, 
                    &LedTaskStk[LED_TASK_STK_SIZE - 1u], 
                    LED_TASK_PRIO, 
                    LED_TASK_PRIO, 
                    &LedTaskStk[0u], 
                    LED_TASK_STK_SIZE, 
                    NULL, 
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
#endif
    OS_EXIT_CRITICAL();
    /* uCOS start */
    OSStart();
}


