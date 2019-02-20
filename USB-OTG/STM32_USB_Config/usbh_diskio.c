
#include "usbh_msc.h"
#include "usb_host.h"
#include "usbh_diskio.h"

DSTATUS USBH_DiskInitialize(void)
{
    return RES_OK;
}

DRESULT USBH_DiskGetStatus(void)
{
    DRESULT res = RES_ERROR;
    MSC_HandleTypeDef *MSC_Handle = hUsbHostFS.pActiveClass->pData;

    if(USBH_MSC_UnitIsReady(&hUsbHostFS, MSC_Handle->current_lun))
    {
        //printf("USB MSC Ready!!\r\n");
        res=RES_OK;
    }
    else
    {
        printf("USB MSC NotReady!!\r\n");
        res=RES_ERROR;
    }
    return res;
}

DRESULT USBH_DiskRead(BYTE *buff, DWORD sector, UINT count)
{
	DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle = hUsbHostFS.pActiveClass->pData;

    if(USBH_MSC_Read(&hUsbHostFS, MSC_Handle->current_lun,sector,buff,count)==USBH_OK)
    {
        res = RES_OK;
    }
    else
    {
        printf("USB Read error!!\r\n");
        USBH_MSC_GetLUNInfo(&hUsbHostFS, MSC_Handle->current_lun, &info);
        switch (info.sense.asc)
        {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;
            default:
                res = RES_ERROR;
                break;
        }
    }
		
	return (res);
}

DRESULT USBH_DiskWrite(const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle = hUsbHostFS.pActiveClass->pData;
    
    if(USBH_MSC_Write(&hUsbHostFS, MSC_Handle->current_lun,sector, (BYTE *)buff,count) == USBH_OK)
    {
        res = RES_OK;
    }
    else
    {
        printf("USB Write error!!\r\n");
        USBH_MSC_GetLUNInfo(&hUsbHostFS, MSC_Handle->current_lun, &info);
        switch (info.sense.asc)
        {
            case SCSI_ASC_WRITE_PROTECTED:
                USBH_ErrLog("USB Disk is Write protected!");
                res = RES_WRPRT;
                break;
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;
            default:
                res = RES_ERROR;
                break;
        }
    }
    return res;
}

DRESULT USBH_DiskIoctl(BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;
    MSC_HandleTypeDef *MSC_Handle = hUsbHostFS.pActiveClass->pData;
    switch(cmd)
    {
        case CTRL_SYNC:
            res=RES_OK;
            break;
        case GET_SECTOR_COUNT : //number of sector
            if(USBH_MSC_GetLUNInfo(&hUsbHostFS, MSC_Handle->current_lun, &info)==USBH_OK)
            {
                *(DWORD*)buff=info.capacity.block_nbr;
                res = RES_OK;
                printf("USB Disk sector nbr:%d\r\n", info.capacity.block_nbr);
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        case GET_SECTOR_SIZE ://sector size
            if(USBH_MSC_GetLUNInfo(&hUsbHostFS, MSC_Handle->current_lun, &info) == USBH_OK)
            {
                *(DWORD*)buff=info.capacity.block_size;
                res = RES_OK;
                printf("USB Disk sector size:%d\r\n", info.capacity.block_size);
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        case GET_BLOCK_SIZE ://block size

            if(USBH_MSC_GetLUNInfo(&hUsbHostFS, MSC_Handle->current_lun, &info)==USBH_OK)
            {
                *(DWORD*)buff = info.capacity.block_size/USB_DEFAULT_BLOCK_SIZE;
                printf("USB Disk block size:%d\r\n", info.capacity.block_size/USB_DEFAULT_BLOCK_SIZE);
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;
        default:
                res = RES_PARERR;
    }
    return res;
}


