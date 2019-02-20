
#include "ucos_ii.h"
#include "ui_event.h"
#include "io_keypad.h"
#include "track_KPD.h"
#include "led.h"

/*  KEY_MAP 
*/
static const IOKeyMap_t IoKeyMap[] = 
{
    {GPIO_KEY0, IO_EVT_RIGHT,   KEY_HOLD_DEFAULT_DELAY, KEY_REPEAT_DEFAULT_DELAY},
    {GPIO_KEY1, IO_EVT_DOWN,    KEY_HOLD_DEFAULT_DELAY, KEY_REPEAT_DEFAULT_DELAY},
    {GPIO_KEY2, IO_EVT_LEFT,    KEY_HOLD_DEFAULT_DELAY, KEY_REPEAT_DEFAULT_DELAY},
    {GPIO_KEY3, IO_EVT_UP,      KEY_HOLD_DEFAULT_DELAY, KEY_REPEAT_DEFAULT_DELAY}
};
#define IO_KEYMAP_SIZE      (sizeof(IoKeyMap) / sizeof(IoKeyMap[0]))

/*  STATIC 
*/
static void IoKeySendEvt(IoEvent_t *evt);

static IoEvent_t IoKeyEvt[IO_KEYMAP_SIZE];
static uint16_t IoHoldCnt[IO_KEYMAP_SIZE];

/************* PUBLIC FUNCTION *******************/
uint8_t KPD_TrackInit(void)
{
    uint8_t i = 0;
    
    for(i = 0; i < IO_KEYMAP_SIZE; i++)
    {
        IoKeyEvt[i].data = 0;
        IoKeyEvt[i].code = 0;
        IoKeyEvt[i].flag = 0;

        IoHoldCnt[i] = 0;
    }

    return (0);
}

void KPD_TrackTask(void *p_arg)
{
    (void)p_arg;
    uint32_t keyScan = 0;
    uint8_t kpd = 0, i = 0;
    signed short idx = -1;
    bool sendEvt = FALSE;

    //static uint8_t tCount = 0;

    while(1)
    {
        keyScan = IoKey_GetToggle();

        for(kpd = 0; kpd < GPIO_KEY_NUM; kpd++)
        {
            sendEvt = FALSE;
            idx = -1;
            if(keyScan & (1<<kpd))
            {
                for(i = 0; i < IO_KEYMAP_SIZE; i++)
                {
                    if(IoKeyMap[i].kpd == kpd)
                    {
                        idx = i;
                        break;
                    }
                }
            }
            //key press
            if(idx >= 0)
            {
            	//printf("key[%d], press\r\n", kpd);
                if(IoHoldCnt[kpd] >= 64999)
                {
                    IoHoldCnt[kpd] = 1;
                }
                IoHoldCnt[kpd]++;

                if((IoHoldCnt[kpd] >= KEY_DOWN_DEFAULT_DELAY) 
                    && (IoHoldCnt[kpd] < IoKeyMap[idx].holdDelay))
                {
                    if(IoKeyEvt[kpd].flag == IO_KEY_NONE)
                    {
                        IoKeyEvt[kpd].code = IoKeyMap[idx].key;
                        IoKeyEvt[kpd].flag = IO_KEY_DOWN;
                        sendEvt = TRUE;
                    }
                }

                if(IoHoldCnt[kpd] >= IoKeyMap[idx].holdDelay)
                {
                    if(IoKeyEvt[kpd].flag == IO_KEY_DOWN)
                    {
                        IoKeyEvt[kpd].flag = IO_KEY_HOLD;
                        sendEvt = TRUE;
                    }
                }

                if((IoHoldCnt[kpd] % IoKeyMap[idx].repeatDelay) == 0)
                {
                    if((IoKeyEvt[kpd].flag == IO_KEY_HOLD) 
                        || (IoKeyEvt[kpd].flag == IO_KEY_REPEAT))
                    {
                        IoKeyEvt[kpd].flag = IO_KEY_REPEAT;
                        sendEvt = TRUE;
                    }
                }
            }
            else
            {
            	//key release
                if(IoKeyEvt[kpd].flag == IO_KEY_DOWN)
                {
                    IoKeyEvt[kpd].flag = IO_KEY_UP_BEFORE_HOLD;
                    sendEvt = TRUE;
                }

                if((IoKeyEvt[kpd].flag == IO_KEY_HOLD) 
                    || (IoKeyEvt[kpd].flag == IO_KEY_REPEAT))
                {
                    IoKeyEvt[kpd].flag = IO_KEY_UP_AFTER_HOLD;
                    sendEvt = TRUE;
                }

                IoHoldCnt[kpd] = 0;
            }

            if(sendEvt)
            {
            	//printf("send kpd[%d]\r\n", kpd);
                IoKeySendEvt(&IoKeyEvt[kpd]);
            }

            if(IoHoldCnt[kpd] == 0)
            {
                IoKeyEvt[kpd].code = 0;
                IoKeyEvt[kpd].flag = IO_KEY_NONE;
                IoKeyEvt[kpd].data = 0;
            }    
        }
#if 0
		if(++tCount == 50)
		{
			tCount = 0;
			LED_Toggle(LED_RED);
		}
#endif 
        /* task delay */
        OSTimeDlyHMSM(0, 0, 0, KEY_TRACK_TICK_PERIOD_MS);
    }
}

/************ PRIVATE FUNCTION *****************/
static void IoKeySendEvt(IoEvent_t *evt)
{
    UiSendIoEvt(evt);
}

