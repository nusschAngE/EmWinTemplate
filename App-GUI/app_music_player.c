
#include "app_music_player.h"

#include "DIALOG.h"
#include "GUI.h"
#include "WM.h"

#include "ui_event.h"


/*********************************************
*       File List
**********************************************/


/*********************************************
*       Play screen
**********************************************/



/*** Music player app ****/
void Musicplayer_Load(void *p_arg)
{
    UiEvent_t UiEvt;

    while(1)
    {

        /* keypad proc */
        if(UiGetUiEvt(&UiEvt) == 0)
        {
            switch (UiEvt.code)
            {
                case UI_EVT_IO(IO_EVT_UP, IO_KEY_DOWN):
                    break;
                case UI_EVT_IO(IO_EVT_DOWN, IO_KEY_DOWN):
                    break;
                case UI_EVT_IO(IO_EVT_LEFT, IO_KEY_DOWN):
                    break;
                case UI_EVT_IO(IO_EVT_RIGHT, IO_KEY_DOWN):
                    break;
                default:
                    break;
            }
        }

        /* delay */
        GUI_Delay(20);
    }
}

