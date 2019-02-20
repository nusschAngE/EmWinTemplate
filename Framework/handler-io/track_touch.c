
#include "stm32h7xx.h"
#include "ucos_ii.h"
#include "touch.h"
#include "track_touch.h"
#include "GUI.h"


/******** PUBLIC FUNCTION *********/

/*  touch track task body
*   !!curren read one point only
*/
void TP_TrackTask(void *p_arg)
{
    bool tpInt = TRUE;
    
    while(1)
    {
        //tpInt = TP_ReadInterrupt();
        if(tpInt) {
            TP_ReadPoints(&tpPoints);
        } else {
            TP_Release(&tpPoints);
        }

        GUI_TOUCH_Exec();
        OSTimeDlyHMSM(0, 0, 0, 20);
    }
}


