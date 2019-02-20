
#include "stm32h7xx.h"
#include "system_conf.h"

#include "delay.h"
#include "printf.h"
#include "mytext.h"
#include "my_malloc.h"
#include "ucos_ii.h"

#include "sdram.h"
#include "led.h"
#include "io_expand.h"
#include "tft_lcd.h"
#include "qspi_flash.h"
#include "touch.h"
#include "io_keypad.h"
#include "ir_keypad.h"
#include "ds18b20.h"

#include "gui_startup.h"

/*  PUBLIC
*/
int main(void)
{
    uint8_t ret = 0, errIndex = 0;
	
    /* mcu configuration */
    System_SysTickInit(1000);
    //HAL_MspInit();
   	System_SCBCacheConfig();
#if 1//!defined(ExtSDRAM_ENABLE)
    System_MPUConfig();
    ExtSDRAM_Init(); 
#endif //ExtSDRAM_ENABLE   	

    /* malloc source init */
    myMalloc_Init(MLCSRC_AXISRAM);
    myMalloc_Init(MLCSRC_AHBSRAM12);
    myMalloc_Init(MLCSRC_AHBSRAM4);
    myMalloc_Init(MLCSRC_DTCM);
    myMalloc_Init(MLCSRC_ITCM);
#if defined(ExtSDRAM_ENABLE)
    myMalloc_Init(MLCSRC_SDRAM);
#endif //ExtSDRAM_ENABLE

    /* TIM delay us/ms */
    ret = DelayModule_Init();
    if(ret != 0) {HANDLE_ERROR();}
    /* USART1 printf */
    ret = PrintfModule_Init();
    if(ret != 0) {HANDLE_ERROR();}

    /* system start */
    printf("\r\nhardware init...\r\n");

    /* hardware init */
    ret = LED_Init();//LED Init
    if(ret != 0) {errIndex = 1; goto Main_ErrHandle;}
    
	ret = TFTLCD_Init();//TFT LCD Init
	if(ret != 0) {errIndex = 2; goto Main_ErrHandle;}
	
    ret = IOEx_Init();//IO Expand Init
    if(ret != 0) {errIndex = 3; goto Main_ErrHandle;}
    
    ret = QFL_Init();//QSPI Flash Init
    if(ret != 0) {errIndex = 4; goto Main_ErrHandle;}
    
    ret = IoKey_Init();//GPIO Key Init
    if(ret != 0) {errIndex = 5; goto Main_ErrHandle;}
    
    ret = IRKey_Init();//IR-remote Key Init
    if(ret != 0) {errIndex = 6; goto Main_ErrHandle;}
    
	ret = TP_Init();//Touch Screen Init
	if(ret != 0) {errIndex = 7; goto Main_ErrHandle;}
    
    ret = HZKFont_Check();//HZK Font Bin
    if(ret != 0) 
    {
        printf("HZK font is not ready, ret = %d\r\n", ret);
    }
    
/*  GUI app system 
*/  
    printf("app start...\r\n");
    GUI_Startup();

Main_ErrHandle:
    printf("handle error, index = %d\r\n", errIndex);
    LED_Onoff(LED_GREEN, FALSE);
    /* while(1) loop */
    while(1)
	{
	    LED_Toggle(LED_GREEN);
		TimDelayMs(200);
	}
	
    /* never reach here */
    //return 0;
}



