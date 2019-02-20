
#ifndef _USBH_DISKIO_H
#define _USBH_DISKIO_H

#include "public.h"
#include "diskio.h"


#define USB_DEFAULT_BLOCK_SIZE 	(512u)



DRESULT USBH_DiskInitialize(void);
DRESULT USBH_DiskGetStatus(void);
DRESULT USBH_DiskRead(BYTE *buff, DWORD sector, UINT count);
DRESULT USBH_DiskWrite(const BYTE *buff, DWORD sector, UINT count);
DRESULT USBH_DiskIoctl(BYTE cmd, void *buff);





#endif


