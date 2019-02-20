#ifndef _SDCARD_H
#define _SDCARD_H

#include "public.h"

//#define SDCARD_DMA_MODE     (1U)

enum
{
	SD_ERROR_NONE = 0,
	SD_ERROR_PWRON,
	SD_ERROR_INITCARD,
	SD_ERROR_CMD,
	SD_ERROR_PROC,
};

enum
{
    SD_RESET                  = ((uint32_t)0x00000000U),  /*!< SD not yet initialized or disabled  */
    SD_READY                  = ((uint32_t)0x00000001U),  /*!< SD initialized and ready for use    */
    SD_TIMEOUT                = ((uint32_t)0x00000002U),  /*!< SD Timeout state                    */
    SD_BUSY                   = ((uint32_t)0x00000003U),  /*!< SD process ongoing                  */
    SD_PROGRAMMING            = ((uint32_t)0x00000004U),  /*!< SD Programming State                */
    SD_RECEIVING              = ((uint32_t)0x00000005U),  /*!< SD Receinving State                 */
    SD_TRANSFER               = ((uint32_t)0x00000006U),  /*!< SD Transfert State                  */
    SD_ERROR                  = ((uint32_t)0x0000000FU)   /*!< SD is in error state                */
};

/* for fatfs */
typedef struct
{
    uint32_t CardType;
    uint32_t CardVersion;

    uint32_t BlockNbr;
    uint32_t BlockSize;
}SDBaseInfo_t;

extern SDBaseInfo_t SDBaseInfo;
/*public function*/
uint8_t  SDCard_Init(void);
uint8_t  SDCard_GetCardState(void);
void SDCard_GetBaseInfo(SDBaseInfo_t *info);
uint8_t SDCard_ReadMultipleBlocks(uint8_t *pBuff, uint32_t BlockAddr, uint32_t BlockNbr);
uint8_t SDCard_WriteMultipleBlocks(uint8_t *pData, uint32_t BlockAddr, uint32_t BlockNbr);




















#endif
