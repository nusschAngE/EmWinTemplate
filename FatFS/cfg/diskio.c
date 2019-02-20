/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

#include "sdcard_diskio.h"
#include "usbh_diskio.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_SDMMC       0
#define DEV_USB         1
#define DEV_NAND        2


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;
	DRESULT res = RES_ERROR;

	switch (pdrv) 
	{
	case DEV_SDMMC:
		res = SD_DiskGetStatus();
        if(res != RES_OK)
        {
            stat |= STA_NODISK;
        }
        
		return (stat);
		
	case DEV_USB:
		res = USBH_DiskGetStatus();
		//printf("USB Disk Status, ret = %d\r\n", result);
        if(res != RES_OK) 
        {
            stat |= STA_NODISK;
        }
		return (stat);
	}
	
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;
	DRESULT res = RES_ERROR;

	switch (pdrv) 
	{
	case DEV_SDMMC:
		res = SD_DiskInitialize();
		if(res != RES_OK)
        {
            stat |= STA_NOINIT;
        }
		return (stat);
		
	case DEV_USB:
		res = USBH_DiskInitialize();
		//printf("USB Disk init, ret = %d\r\n", result);
		if(res != RES_OK)
        {
            stat |= STA_NOINIT;
        }
        return (stat);
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;

	switch (pdrv) 
	{
	case DEV_SDMMC:
		res = SD_DiskRead(buff, sector, count);
		return (res);
		
	case DEV_USB:
		res = USBH_DiskRead(buff, sector, count);
		return (res);
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) 
	{
	case DEV_SDMMC :
		res = SD_DiskWrite(buff, sector, count);
		return (res);
		
	case DEV_USB:
		res = USBH_DiskWrite(buff, sector, count);
		return (res);
	}
		
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (pdrv) 
	{
	case DEV_SDMMC :
		res = SD_DiskIoctl(cmd, buff);
		return (res);
		
	case DEV_USB:
		res = USBH_DiskIoctl(cmd, buff);
		return (res);
	}

	return RES_PARERR;
}

DWORD get_fattime(void)
{
	return 0;
}

