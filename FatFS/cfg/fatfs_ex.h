
#ifndef _FATFS_EX_H
#define _FATFS_EX_H

#include "public.h"
#include "ff.h"

/* driver */
#define FS_DRV_NONE     (255)
#define FS_DRV_SDCARD   (0)
#define FS_DRV_USBMSC   (1)
#define FS_DRV_NAND     (2)

#define SdFsDrv         ((const TCHAR*)("0:"))
#define UsbFsDrv        ((const TCHAR*)("1:"))
#define NandFsDrv       ((const TCHAR*)("2:"))

typedef enum
{
    /* bin */
    T_BIN = 0,
    T_FON,
    /* doc */
    T_TXT,
    /* audio */
    T_WAV,
    T_MP3,
    /* null */
    T_UNSUPRT
}FTYPE;

/*************** PUBLIC FUNCTION *****************/
extern FATFS SDFatFS;
extern FATFS USBFatFS;
extern FATFS NANDFatFS;
extern FATFS **tmpFS;

extern bool IsSDMount;
extern bool IsUSBMount;
extern bool IsNANDMount;

extern FRESULT SDCard_Mount(void);
extern FTYPE f_TypeTell(TCHAR* fname);
extern uint16_t f_GetFilePath(TCHAR *path, uint16_t pSize, TCHAR *dirName, TCHAR *fileName);
extern uint16_t f_GetDirName(TCHAR *path, uint16_t pSize, TCHAR *dirName);
extern uint16_t f_GetFileName(TCHAR *path, uint16_t pSize, TCHAR *fileName);
#endif

