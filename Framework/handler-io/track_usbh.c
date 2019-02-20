
#include "stm32h7xx.h"
#include "ucos_ii.h"
#include "fatfs_ex.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_conf.h"
#include "usb_host.h"
#include "track_usbh.h"


/********** PRIVATE FUNCTION ************/

static void USBH_MyProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    /* USER CODE BEGIN CALL_BACK_1 */
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
    {
        printf("USBH Configuration!!\r\n");
    }
    break;

    case HOST_USER_DISCONNECTION:
    {
        Appli_state = APPLICATION_DISCONNECT;
        printf("USBH Disconnected Process!!\r\n");
        switch(phost->pActiveClass->ClassCode)
        {
        case USB_MSC_CLASS:
            {                                
                f_unmount(UsbFsDrv);
                IsUSBMount = FALSE;
            }
            break;
        default:
            break;
        }
    }
    break;

    case HOST_USER_CLASS_ACTIVE:
    {
        Appli_state = APPLICATION_READY;
        printf("USBH Class Active Process!!\r\n");
        switch(phost->pActiveClass->ClassCode)
        {
        case USB_MSC_CLASS:
            {
                FRESULT fret = FR_OK;
            
                fret = f_mount(&USBFatFS, UsbFsDrv, 1);
                if(fret == FR_OK) {
                    printf("f_mount() success!!\r\n");
                    IsUSBMount = TRUE;
                } else {
                    printf("f_mount() error, fret = %d!!\r\n", fret);
                    IsUSBMount = FALSE;
                }
            }
            break;
        default:
            break;
        }
    }
    break;

    case HOST_USER_CONNECTION:
    {
        Appli_state = APPLICATION_START;
        printf("USB Connected Process!!\r\n");
    }
    break;

    default:break;
    }
    /* USER CODE END CALL_BACK_1 */    
}

/****** PUBLIC FUNCTION *********/

void USBH_TrackInit(void)
{  
    /* Init host Library, add supported class and start the library. */
    USBH_Init(&hUsbHostFS, USBH_MyProcess, HOST_FS);

    //USBH_RegisterClass(&hUsbHostFS, USBH_AUDIO_CLASS);
    //USBH_RegisterClass(&hUsbHostFS, USBH_CDC_CLASS);
    USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS);
    //USBH_RegisterClass(&hUsbHostFS, USBH_HID_CLASS);
    //USBH_RegisterClass(&hUsbHostFS, USBH_MTP_CLASS);

    USBH_Start(&hUsbHostFS);
	HAL_PWREx_EnableUSBVoltageDetector();
}

/**** USB Track task body *****/


void USBH_TrackTask(void *p_arg)
{
    (void)p_arg;

    while(1)
    {
        USBH_Process(&hUsbHostFS);
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
}






