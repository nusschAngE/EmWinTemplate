
#include "stm32h7xx.h"
#include "ucos_ii.h"
#include "my_malloc.h"
#include "delay.h"
#include "GUI.h"
#include "DIALOG.h"

#include "led.h"
#include "wifi_esp8266.h"
#include "uart_atk.h"

#include "ui_event.h"
#include "app_home.h"
#include "app_handle.h"
#include "app_esp8266_template.h"

#define SERVER_IP_ADDR      "192.168.1.102"
#define SERVER_IP_PORT      "8080"

#define ESP_IP_ADDR         "192.168.1.103"
#define ESP_IP_PORT         "8086"

#define AP_SSID             "TP-LINK_F59A"
#define AP_PASSWD           "dotabear1233"

/*  */
#define ESP_ID_WINDOW_0 (GUI_ID_USER + 0x00)
#define ESP_ID_TEXT_0   (GUI_ID_USER + 0x01)
#define ESP_ID_TEXT_1   (GUI_ID_USER + 0x08)
#define ESP_ID_EDIT_0   (GUI_ID_USER + 0x09)
#define ESP_ID_TEXT_2   (GUI_ID_USER + 0x0B)
#define ESP_ID_EDIT_1   (GUI_ID_USER + 0x0C)
#define ESP_ID_TEXT_3   (GUI_ID_USER + 0x0D)
#define ESP_ID_EDIT_2   (GUI_ID_USER + 0x0E)
#define ESP_ID_TEXT_4   (GUI_ID_USER + 0x0F)
#define ESP_ID_EDIT_3   (GUI_ID_USER + 0x10)
#define ESP_ID_TEXT_5   (GUI_ID_USER + 0x11)
#define ESP_ID_EDIT_4   (GUI_ID_USER + 0x12)
#define ESP_ID_TEXT_6   (GUI_ID_USER + 0x13)
#define ESP_ID_EDIT_5   (GUI_ID_USER + 0x14)
#define ESP_ID_TEXT_7   (GUI_ID_USER + 0x15)
#define ESP_ID_EDIT_6   (GUI_ID_USER + 0x16)
#define ESP_ID_BUTTON_0 (GUI_ID_USER + 0x17)
#define ESP_ID_BUTTON_1 (GUI_ID_USER + 0x18)

/* led status */
#define LED_RED_XPOS    (105)
#define LED_RED_YPOS    (282)

#define LED_GREEN_XPOS  (205)
#define LED_GREEN_YPOS  (282)

/* UI Layout */
static const GUI_WIDGET_CREATE_INFO _espDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ESP_ID_WINDOW_0, 0, 0, 480, 800, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Text0", ESP_ID_TEXT_0, 120, 5, 240, 20, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text1", ESP_ID_TEXT_1, 20, 28, 40, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit0", ESP_ID_EDIT_0, 62, 28, 96, 16, 0, 0xc, 0 },
  { TEXT_CreateIndirect, "Text2", ESP_ID_TEXT_2, 165, 28, 32, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit1", ESP_ID_EDIT_1, 192, 28, 136, 16, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text3", ESP_ID_TEXT_3, 341, 28, 40, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit2", ESP_ID_EDIT_2, 380, 28, 40, 16, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text4", ESP_ID_TEXT_4, 20, 50, 68, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit3", ESP_ID_EDIT_3, 89, 50, 136, 16, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text5", ESP_ID_TEXT_5, 235, 50, 42, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit4", ESP_ID_EDIT_4, 274, 50, 40, 16, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text6", ESP_ID_TEXT_6, 20, 90, 80, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit5", ESP_ID_EDIT_5, 20, 110, 440, 64, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "Text7", ESP_ID_TEXT_7, 20, 184, 80, 16, 0, 0x64, 0 },
  { EDIT_CreateIndirect, "Edit6", ESP_ID_EDIT_6, 20, 200, 440, 64, 0, 0x64, 0 },
  { BUTTON_CreateIndirect, "Button0", ESP_ID_BUTTON_0, 20, 280, 80, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Button1", ESP_ID_BUTTON_1, 120, 280, 80, 20, 0, 0x0, 0 },};

/* LED Control Value */
static bool ledGreen = TRUE;
static WM_HWIN hEspWin;

/* create window & event process */
static void _cbESPDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  //GUI_COLOR oldColor;

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    // Initialization of 'Window'
    hEspWin = hItem = pMsg->hWin;
    WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0x009E9E9E));
    // Initialization of 'Text0' TITLE
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_0);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
    TEXT_SetFont(hItem, GUI_FONT_8X16);
    TEXT_SetText(hItem, "ESP8266 Template");
    // Initialization of 'Text1' MAC
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    TEXT_SetText(hItem, "MAC :");
    // Initialization of 'Edit0'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_0);
    EDIT_SetText(hItem, "NULL");
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text2'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_2);
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    TEXT_SetText(hItem, "IP :");
    // Initialization of 'Edit1'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_1);
    EDIT_SetText(hItem, ESP_IP_ADDR);
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text3'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_3);
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    TEXT_SetText(hItem, "Port :");
    // Initialization of 'Edit2'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_2);
    EDIT_SetText(hItem, "0");
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text4'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_4);
    TEXT_SetText(hItem, "Server IP :");
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    // Initialization of 'Edit3'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_3);
    //EDIT_SetText(hItem, SERVER_IP_ADDR);
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text5'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_5);
    TEXT_SetText(hItem, "Port :");
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    // Initialization of 'Edit4'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_4);
    //EDIT_SetText(hItem, SERVER_IP_PORT);
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text6'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_6);
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00D73545));
    TEXT_SetText(hItem, "Receive :");
    // Initialization of 'Edit5'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_5);
    //EDIT_SetText(hItem, "Wait ESP8266 Connect...");
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Text7'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_TEXT_7);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x003613C4));
    TEXT_SetFont(hItem, GUI_FONT_16_ASCII);
    TEXT_SetText(hItem, "Send :");
    // Initialization of 'Edit6'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_EDIT_6);
    //EDIT_SetText(hItem, "NULL");
    EDIT_SetFont(hItem, GUI_FONT_16_ASCII);
    EDIT_SetFocussable(hItem, 0);
    // Initialization of 'Button0'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_BUTTON_0);
    BUTTON_SetText(hItem, "LED ON");
    // Initialization of 'Button1'
    hItem = WM_GetDialogItem(pMsg->hWin, ESP_ID_BUTTON_1);
    BUTTON_SetText(hItem, "LED OFF");
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ESP_ID_EDIT_0: // Notifications sent by 'Edit0'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_1: // Notifications sent by 'Edit1'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_2: // Notifications sent by 'Edit2'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_3: // Notifications sent by 'Edit3'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_4: // Notifications sent by 'Edit4'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_5: // Notifications sent by 'Edit5'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_EDIT_6: // Notifications sent by 'Edit6'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        break;
      }
      break;
    case ESP_ID_BUTTON_0: // Notifications sent by 'Button0'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        if(ledGreen == FALSE) 
        {
            LED_Onoff(LED_GREEN, TRUE);
            ledGreen = TRUE;
            ESP8266_PutMessageSendPool((uint8_t *)"LED:ON", strlen("LED:ON"));
        } 
        break;
      }
      break;
    case ESP_ID_BUTTON_1: // Notifications sent by 'Button1'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        break;
      case WM_NOTIFICATION_RELEASED:
        if(ledGreen == TRUE) 
        {
            LED_Onoff(LED_GREEN, FALSE);
            ledGreen = FALSE;
            ESP8266_PutMessageSendPool((uint8_t *)"LED:OFF", strlen("LED:OFF"));
        }
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

static WM_HWIN ESPCreateWindow(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_espDialogCreate, GUI_COUNTOF(_espDialogCreate), _cbESPDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

/***********************************************
*       ESP8266 Template App Content
***********************************************/
#define ESP_Delay       OSDelay

static uint8_t ESPBuffer[ESP_MAX_MESGSIZE];
static bool    IsEspConnected = FALSE;
static uint8_t ESPInitProc = 0;

static bool EspTemp_EvtHandler(void *evt, void *p_arg)
{
    (void)p_arg;
    uint8_t ret = 0;
    uint32_t mesgLen = 0;
    WM_HWIN hItem;
    UiEvent_t *uiEvt = (UiEvent_t *)evt;
    bool handled = FALSE;

    switch(ESPInitProc)
    {
        case 0://AT Test
            ret = ESP8266_TestATCmd();
            if(ret != ESP_RES_OK) {printf("ESP8266_TestATCmd(), ret = %d\r\n", ret);}
            ESPInitProc++;
            ESP_Delay(100);
            break;
        case 1:
            ret = ESP8266_EchoOnoff(0);
            if(ret != ESP_RES_OK) {printf("ESP8266_EchoOnoff(), ret = %d\r\n", ret);}
            ESPInitProc++;
            ESP_Delay(100);
            break;
        case 2:
            //ret = ESP8266_JoinAP((uint8_t*)AP_SSID, (uint8_t*)AP_PASSWD);
            //if(ret != ESP_RES_OK) {printf("ESP8266_JoinAP() error, ret = %d\r\n", ret);}
            ESPInitProc++;
            //ESP_Delay(500);
            break;
        case 3:
            ret = ESP8266_SetStationIP(ESP_IP_ADDR);
            if(ret != ESP_RES_OK) {printf("ESP8266_SetStationIP() error, ret = %d\r\n", ret);}
            ESPInitProc++;
            ESP_Delay(500);
            break;
        case 4:
            ret = ESP8266_IPConnect0("TCP", SERVER_IP_ADDR, (uint16_t)atoi(SERVER_IP_PORT));
            if(ret != ESP_RES_OK) 
            {
                printf("ESP8266_IPConnect0() error, ret = %d\r\n", ret);
                //Server IP
                hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_3);
                EDIT_SetText(hItem, "NULL");
                //Server Port
                hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_4);
                EDIT_SetText(hItem, "0");
                IsEspConnected = FALSE;
            } 
            else 
            {
                //Server IP
                hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_3);
                EDIT_SetText(hItem, SERVER_IP_ADDR);
                //Server Port
                hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_4);
                EDIT_SetText(hItem, SERVER_IP_PORT);
                IsEspConnected = TRUE;
            }
            ESPInitProc++;
            ESP_Delay(500);
            break;
        default:
            break;
    }

    /* UI event proc */
    if(uiEvt)
    {
        switch (uiEvt->code)
        {
            case UI_EVT_IO(IO_EVT_UP, IO_KEY_UP_BEFORE_HOLD):
                /* go back to home */
                AppMenu_Load(NULL);
                handled = TRUE;
                break;
            default:
                handled = FALSE;
                break;
        }
    }

    /* TCP/UDP message */
    mesgLen = ESP8266_GetMessageRecv(ESPBuffer);
    if(mesgLen)
    {
    	ESPBuffer[mesgLen] = 0;
        hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_5);
        EDIT_SetText(hItem, (const char*)ESPBuffer);
        
        if(myStrstr((char *)ESPBuffer, "LED ON") != NULL)
        {
            if(ledGreen == FALSE) 
            {
                LED_Onoff(LED_GREEN, TRUE);
                ledGreen = TRUE;
                ESP8266_PutMessageSendPool((uint8_t *)"LED:ON", strlen("LED:ON"));
            }
        }

        if(myStrstr((char *)ESPBuffer, "LED OFF") != NULL)
        {
            if(ledGreen == TRUE) 
            {
                LED_Onoff(LED_GREEN, FALSE);
                ledGreen = FALSE;
                ESP8266_PutMessageSendPool((uint8_t *)"LED:OFF", strlen("LED:OFF"));
            }
        }

        if(myStrstr((char *)ESPBuffer, "LED T") != NULL)
        {
            uint32_t tick = atoi(strchr((char *)ESPBuffer, 'T')+1);
            printf("LED blink tick = %d\r\n", tick);
        }

        myMemset(ESPBuffer, 0, sizeof(ESPBuffer));
    }

#if 1
    mesgLen = ESP8266_GetMessageSend(ESPBuffer);
    if(mesgLen && IsEspConnected)
    {
    	ESPBuffer[mesgLen] = 0;
    	printf("IP Send[%d] : %s\r\n", mesgLen, ESPBuffer);
        ret = ESP8266_IPSend0(ESPBuffer, mesgLen, 0xffff);
		if(ret != ESP_RES_OK) {printf("ESP8266_IPSend0, ret = %d\r\n", ret);}
        ESPBuffer[mesgLen] = '\0';
        hItem = WM_GetDialogItem(hEspWin, ESP_ID_EDIT_6);
        EDIT_SetText(hItem, (const char*)ESPBuffer);
    }
#endif

    return (handled);
}

static void EspTemp_Unload(void)
{
    ESP8266_DeInit();
    //GUI_Clear();
    //GUI_CURSOR_Hide();
    //WM_DeleteWindow(hEspWin);
    //WM_Paint(WM_HBKWIN);
}

static AppContent_t App_EspTemp = 
{
    EspTemp_EvtHandler,
    EspTemp_Unload,
};

/***********************************************
*       ESP8266 Template App Load
***********************************************/
static void EspTemp_Init(void)
{
    uint8_t ret = 0;
        
    /* UI Create */
    GUI_CURSOR_Hide();
    ESPCreateWindow();
    GUI_Exec();

    ESPInitProc = 0;
    IsEspConnected = FALSE;
    myMemset(ESPBuffer, 0, sizeof(ESPBuffer));
    /* ESP8266 Init */
    ret = ESP8266_Init();
    if(ret != ESP_RES_OK) {printf("ESP8266_Init() error, ret = %d\r\n", ret);}
    
    /* update TEXT */
}

void EspTemp_Load(void *p_arg)
{
    (void)p_arg;

    EspTemp_Init();
    UiApp_Register(&App_EspTemp, APP_LV_PLAY);
}


