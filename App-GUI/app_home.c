
#include "stm32h7xx.h"
#include "public.h"

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"

#include "app_handle.h"
#include "ui_event.h"

#include "app_home.h"
#include "app_esp8266_template.h"
#include "wifi_app_icon.h"

/* app list */
typedef struct
{
    uint16_t    idx;
    const GUI_BITMAP *icon;
    const char  text[32];
    void      (*AppLoad)(void *p_arg);
    void       *p_arg;
}AppList_t;

static const AppList_t applist[] = 
{
    {0, &bmwifi_app_icon, "wifi test0", EspTemp_Load, NULL},
    {0, &bmwifi_app_icon, "wifi test1", EspTemp_Load, NULL},
    {0, &bmwifi_app_icon, "wifi test2", EspTemp_Load, NULL},
    {0, &bmwifi_app_icon, "wifi test3", EspTemp_Load, NULL},
};

#define APP_LIST_NBR    (sizeof(applist)/sizeof(applist[0]))

/* select app idx */
static int appSelIdx = -1;

/*  */
#define APP_ID_WINDOW_0     (GUI_ID_USER + 0x00)
#define APP_ID_TEXT_0       (GUI_ID_USER + 0x01)
#define APP_ID_ICONVIEW_0   (GUI_ID_USER + 0x02)

/* GUI Layout */
static const GUI_WIDGET_CREATE_INFO _AppDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", APP_ID_WINDOW_0, 0, 0, 480, 800, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text", APP_ID_TEXT_0, 80, 10, 320, 16, 0, 0x64, 0 },
  { ICONVIEW_CreateIndirect, "Iconview", APP_ID_ICONVIEW_0, 20, 40, 440, 720, 0, 0x0, 0 },
};

static WM_HWIN hAppWin;

static void _cbAppDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  int     idx;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    // Initialization of 'Window'
    hAppWin = hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, GUI_DARKGRAY);
    // Initialization of 'Text'
    hItem = WM_GetDialogItem(pMsg->hWin, APP_ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_8X16);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetText(hItem, "App Menu");
    // Initialization of 'IconView'
    hItem = WM_GetDialogItem(pMsg->hWin, APP_ID_ICONVIEW_0);
    //ICONVIEW_SetBkColor(hItem, ICONVIEW_CI_BK, GUI_LIGHTGRAY);
    ICONVIEW_SetBkColor(hItem, ICONVIEW_CI_BK, GUI_WHITE);
    ICONVIEW_SetBkColor(hItem, ICONVIEW_CI_SEL, GUI_LIGHTGRAY|0x70000000);
    ICONVIEW_SetFont(hItem, GUI_FONT_6X8_ASCII);
    ICONVIEW_SetTextAlign(hItem, GUI_TA_CENTER);
    ICONVIEW_SetWrapMode(hItem, GUI_WRAPMODE_WORD);
    ICONVIEW_SetIconAlign(hItem, ICONVIEW_IA_LEFT|ICONVIEW_IA_TOP);
    ICONVIEW_SetSpace(hItem, GUI_COORD_X, 76);
    ICONVIEW_SetSpace(hItem, GUI_COORD_Y, 76);
    //add item
    for(idx = 0; idx < APP_LIST_NBR; idx++) {
        ICONVIEW_AddBitmapItem(hItem, applist[idx].icon, applist[idx].text);
    }
    
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case APP_ID_ICONVIEW_0: // Notifications sent by 'Iconview'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        {
            
        }
        break;
      case WM_NOTIFICATION_RELEASED:
        {
            hItem = WM_GetDialogItem(pMsg->hWin, APP_ID_ICONVIEW_0);
            //appListIdx = ICONVIEW_GetSel(hItem);
            appSelIdx = ICONVIEW_GetReleasedItem(hItem);
            printf("T-App[%d] selected\r\n", appSelIdx);
        }
        break;
      case WM_NOTIFICATION_MOVED_OUT:
        break;
      case WM_NOTIFICATION_SCROLL_CHANGED:
        break;
      case WM_NOTIFICATION_SEL_CHANGED:
        break;
      }
      break;
    }
    break;
#if 0    
  case WM_KEY:
    {
        switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key)
        {
            case GUI_KEY_LEFT:
                {
                }
                break;
            case GUI_KEY_RIGHT:
                {
                }
                break;
        }
    }
    break;
#endif    
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

static WM_HWIN AppMenuCreateWindow(void)
{
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_AppDialogCreate, GUI_COUNTOF(_AppDialogCreate), _cbAppDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

/******** App Content *******/
static bool AppMenu_EvtHandler(void *evt, void *p_arg)
{
    WM_HWIN hItem;
    UiEvent_t *uiEvt = (UiEvent_t *)evt;
    bool handled = FALSE;
#if 0
    /* UI event proc */
    if(uiEvt)
    {
        switch (uiEvt->code)
        {
            case UI_EVT_IO(IO_EVT_UP, IO_KEY_UP_BEFORE_HOLD):
                handled = TRUE;
                break;
                
            case UI_EVT_IO(IO_EVT_DOWN, IO_KEY_UP_BEFORE_HOLD):
                //select
                //GUI_StoreKeyMsg(GUI_KEY_ENTER, 1);
                hItem = WM_GetDialogItem(hAppWin, APP_ID_ICONVIEW_0);
                appIdx = ICONVIEW_GetSel(hItem);
                printf("K-App[%d] selected\r\n", appIdx);
                handled = TRUE;
                break;
                
            case UI_EVT_IO(IO_EVT_LEFT, IO_KEY_UP_BEFORE_HOLD):
                GUI_StoreKeyMsg(GUI_KEY_LEFT, 1);
                printf("key pad LEFT\r\n");
                handled = TRUE;
                break;
                
            case UI_EVT_IO(IO_EVT_RIGHT, IO_KEY_UP_BEFORE_HOLD):
                GUI_StoreKeyMsg(GUI_KEY_RIGHT, 1);
                printf("key pad RIGHT\r\n");
                handled = TRUE;
                break;
                
            default:
                handled = FALSE;
                break;
        }
    }
#endif
    /* load app? */
    if((appSelIdx >= 0) && (appSelIdx < APP_LIST_NBR))
    {
        applist[appSelIdx].AppLoad(applist[appSelIdx].p_arg);
        appSelIdx = -1;
    }
	
	return handled;
}

static void AppMenu_Unload(void)
{
    //GUI_Clear();
    //GUI_CURSOR_Hide();
    //WM_DeleteWindow(hAppWin);
    //WM_Paint(WM_HBKWIN);
}

static AppContent_t App_AppMenu = 
{
    AppMenu_EvtHandler,
    AppMenu_Unload,
};

/******** App Load  *********/
static void AppMenu_Init(void)
{
    appSelIdx = -1;

    GUI_CURSOR_Hide();
    AppMenuCreateWindow();
    GUI_Exec();
}

void AppMenu_Load(void * p_arg)
{
    (void)p_arg;
    
    AppMenu_Init();
    UiApp_Register(&App_AppMenu, APP_LV_PLAY);
}

