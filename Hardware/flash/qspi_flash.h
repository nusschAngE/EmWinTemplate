
#ifndef _QSPI_FLASH_H
#define _QSPI_FLASH_H

#include "public.h"

#define QFL_MEMSIZE         ((uint32_t)0x2000000) //32MByte
#define QFL_PAGESIZE        ((uint32_t)0x100)//256Byte
#define QFL_SECTORSIZE      ((uint32_t)0x1000)//4KBtye
#define QFL_BLOCKSIZE       ((uint32_t)0x10000)//64KByte

#define QFL_PAGENBR         (QFL_MEMSIZE / QFL_PAGESIZE)
#define QFL_SECTORNBR       (QFL_MEMSIZE / QFL_SECTORSIZE)
#define QFL_BLOCKNBR        (QFL_MEMSIZE / QFL_BLOCKSIZE)

/* QSPI Flash using SPI mode */
enum
{
    QFL_ITFACE_SPI     = 0,
    QFL_ITFACE_DUALSPI,
    QFL_ITFACE_QUADSPI,

    QFL_ITFACE_UNKNOW,
};

/* memory manage */

/* font
*/
#define HZKFONT_ADDR          	(25 * 1024 * 1024)//start at 25MB offset
#define HZKFONT_SIZE          	((6 * 1024 * 1024) + (512 * 1024))//total used 6.5MB
#define HZKFONT_START_SECTOR	(HZKFONT_ADDR / QFL_SECTORSIZE)
#define HZKFONT_SECTORS       	(HZKFONT_SIZE / QFL_SECTORSIZE)

/* IAP
*/
//IAP Bin Info[version, bin size, checkSum]
#define IAPINFO_ADDR            ((20 * 1024 * 1024) + (768 * 1024))
#define IAPINFO_SIZE            (256)
#define IAPINFO_START_SECTOR    (IAPINFO_ADDR / QFL_SECTORSIZE)
#define IAPINFO_SECTORS         (IAPINFO_SIZE / QFL_SECTORSIZE)
//this section is used for save the update bin
#define IAP0_ADDR               (IAPINFO_ADDR + IAPINFO_SIZE)
#define IAP0_SIZE               (2 * 1024 * 1024)
#define IAP0_START_SECTOR       (IAP0_ADDR / QFL_SECTORSIZE)
#define IAP0_SECTORS       	    (IAP0_SIZE / QFL_SECTORSIZE)
//this section is used for save the backup bin
#define IAP1_ADDR               (IAP1_ADDR + IAP0_SIZE)
#define IAP1_SIZE               (2 * 1024 * 1024)
#define IAP1_START_SECTOR       (IAP1_ADDR / QFL_SECTORSIZE)
#define IAP1_SECTORS       	    (IAP1_SIZE / QFL_SECTORSIZE)

/**************************** PUBLIC FUNCTION *******************************/

/* Flash read internal process */
typedef void(*cycle)(uint8_t data);//

/* public function */
uint8_t QFL_Init(void);
uint8_t QFL_EraseSector(uint32_t addr);
uint8_t QFL_EraseSectors(uint32_t StartSector, uint32_t SectorNbr);
uint8_t QFL_Read(uint8_t *buff, uint32_t addr, uint16_t size);
uint8_t QFL_ReadWithCycle(cycle cycleFunc, uint32_t addr, uint16_t size);
/* SPI Flash write, not check */
uint8_t QFL_WriteNoChk(uint8_t *buff, uint32_t addr, uint32_t size);
/* SPI Flash write with check*/
uint8_t QFL_WriteWithChk(uint8_t *buff, uint32_t addr, uint32_t size);

#endif

