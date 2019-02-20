
#include "qspi_flash.h"
#include "font.h"
#include "ff.h"
#include "my_malloc.h"

/*  Update Font 
**/
enum
{
    FONT_UPDATE_UNIGBK = 0,
    FONT_UPDATE_HZK12,
    FONT_UPDATE_HZK16,
    FONT_UPDATE_HZK24,
    FONT_UPDATE_HZK32,

    FONT_UPDATE_NUM
};

static const char *FontUpdatePath[] = 
{
    "/SYSTEM/FONT/HZK/UNIGBK.BIN",	//UNIGBK.BIN
    "/SYSTEM/FONT/HZK/GBK12.FON",	//GBK12.FON
    "/SYSTEM/FONT/HZK/GBK16.FON",	//GBK16.FON
    "/SYSTEM/FONT/HZK/GBK24.FON",	//GBK24.FON
    "/SYSTEM/FONT/HZK/GBK32.FON",	//GBK32.FON
    NULL,                           //INFO
};

/*  update Font data, need file system.
*   save this data to SPI Flash
*   
*   @return :   0 -> ok, 
*               1 -> file error
*               2 -> font type error
*               3 -> write error
*               4 -> read file error
*               5 -> buffer error
*/
static uint8_t HZKFont_Write(HzkFontInfo_t *FontInfo, uint8_t FontUpdate, FIL *FontFIL)
{
    uint32_t br = 0, alignChk = 0;

    uint32_t beginAddr = 0, ofs = 0;
    uint8_t ret = 0;//flash return status
    uint8_t *tempBuff = NULL;

    if(FontFIL->obj.objsize == 0) {
        return (1);
    }
    
    tempBuff = (uint8_t *)myMalloc(MLCSRC_AXISRAM, QFL_SECTORSIZE);
    if(tempBuff == NULL) {
        return 2;
    }
    
    switch (FontUpdate)
    {
        case FONT_UPDATE_UNIGBK:
            FontInfo->unigbkAddr = HZKFONT_ADDR + sizeof(HzkFontInfo_t);
            alignChk = FontInfo->unigbkAddr % 256;
            if(FontInfo->unigbkAddr % 256)
            {
                alignChk = FontInfo->unigbkAddr/256 + 1;
                FontInfo->unigbkAddr = alignChk*256;
            }
            FontInfo->unigbkSize = FontFIL->obj.objsize;
            beginAddr = FontInfo->unigbkAddr;
            break;
        case FONT_UPDATE_HZK12:
            FontInfo->hzk12Addr = FontInfo->unigbkAddr + FontInfo->unigbkSize;
            alignChk = FontInfo->hzk12Addr % 256;
            if(FontInfo->hzk12Addr % 256)
            {
                alignChk = FontInfo->hzk12Addr/256 + 1;
                FontInfo->hzk12Addr = alignChk*256;
            }
            FontInfo->hzk12Size = FontFIL->obj.objsize;
            beginAddr = FontInfo->hzk12Addr;
            break;
        case FONT_UPDATE_HZK16:
            FontInfo->hzk16Addr = FontInfo->hzk12Addr + FontInfo->hzk12Size;
            alignChk = FontInfo->hzk16Addr % 256;
            if(FontInfo->hzk16Addr % 256)
            {
                alignChk = FontInfo->hzk16Addr/256 + 1;
                FontInfo->hzk16Addr = alignChk*256;
            }
            FontInfo->hzk16Size = FontFIL->obj.objsize;
            beginAddr = FontInfo->hzk16Addr;
            break;
        case FONT_UPDATE_HZK24:
            FontInfo->hzk24Addr = FontInfo->hzk16Addr + FontInfo->hzk16Size;
            alignChk = FontInfo->hzk24Addr % 256;
            if(FontInfo->hzk24Addr % 256)
            {
                alignChk = FontInfo->hzk24Addr/256 + 1;
                FontInfo->hzk24Addr = alignChk*256;
            }
            FontInfo->hzk24Size = FontFIL->obj.objsize;
            beginAddr = FontInfo->hzk24Addr;
            break;
        case FONT_UPDATE_HZK32:
            FontInfo->hzk32Addr = FontInfo->hzk24Addr + FontInfo->hzk24Size;
            alignChk = FontInfo->hzk32Addr % 256;
            if(FontInfo->hzk32Addr % 256)
            {
                alignChk = FontInfo->hzk32Addr/256 + 1;
                FontInfo->hzk32Addr = alignChk*256;
            }
            FontInfo->hzk32Size = FontFIL->obj.objsize;
            beginAddr = FontInfo->hzk32Addr;
            break;
    }

    while(1)
    {
        br = 0;
        myMemset(tempBuff, 0, QFL_SECTORSIZE);
        /* read file */
        ret = f_read(FontFIL, tempBuff, QFL_SECTORSIZE, &br);
        if((ret != FR_OK) || (br == 0)) 
        {
            ret = 4;
            break;
        }

        /* write data to flash */
        ret = QFL_WriteNoChk(tempBuff, beginAddr + ofs, br);
        if(ret != 0)
        {
            ret = 3;
            break;
        }

        ofs += br;
        if(br != QFL_SECTORSIZE) 
        {
            ret = 0;
            break;//done
        }
    }

    myFree(MLCSRC_AXISRAM, tempBuff);
    return (ret);
}

/*
*   updata hzk font data.
*   mount disk before call this func.
*
*   @para drv : disk drive["0:" sd, "1:" usb, "2:" nand]
*
*   @return  :  0 -> ok, >0 error
*/
uint8_t HZKFont_Update(const uint8_t *drv)
{
    FIL *FontFIL;
    uint8_t ret = 0, result = 0, update = 0;
    uint8_t tmpBuff[64];

    printf("\r\n--- Font HZK Update start -----------------------\r\n");

    FontFIL = (FIL *)myMalloc(MLCSRC_AXISRAM, sizeof(FIL));
    if(FontFIL == NULL) 
    {
        //printf("font update error, FIL malloc error!\r\n");
        result = 1;
        goto HZKUpdataEnd;
    }

    for(update = 0; update < FONT_UPDATE_NUM; update++)
    {
        myMemset(tmpBuff, 0, sizeof(tmpBuff));
        myStrcpy((char *)tmpBuff, (char *)drv);
        myStrcat((char *)tmpBuff, (char *)FontUpdatePath[update]);

		printf("source bin checking : %s\r\n", tmpBuff);
        ret = f_open(FontFIL, (const TCHAR*)tmpBuff, FA_READ|FA_OPEN_EXISTING);
        if(ret != FR_OK) 
        {
            //printf("font update error, f_open(bin) ret = %d\r\n", ret);
            result = 2;
            goto HZKUpdataEnd;
        }
        
        f_close(FontFIL);
    }

    //lcd debug out : flash erase
    ret = QFL_EraseSectors(HZKFONT_START_SECTOR, HZKFONT_SECTORS);
    if(ret != 0) 
    {
        //printf("font update error, flash erase error!\r\n");
        result = 3;
        goto HZKUpdataEnd;
    }

    //lcd debug out : update processing
    for(update = 0; update < FONT_UPDATE_NUM; update++)
    {
        myMemset(tmpBuff, 0, sizeof(tmpBuff));
        myStrcpy((char *)tmpBuff, (char *)drv);
        myStrcat((char *)tmpBuff, (char *)FontUpdatePath[update]);

		printf("font updating : %s\r\n", FontUpdatePath[update]);
        ret = f_open(FontFIL, (const TCHAR*)tmpBuff, FA_READ|FA_OPEN_EXISTING);
        if(ret == FR_OK)
        {
            //printf("font update error, f_open(bin) ret = %d\r\n", ret);
            ret = HZKFont_Write(&HzkFontInfo, update, FontFIL);
            if(ret != 0)
            {
                f_close(FontFIL);
                result = 6;
                goto HZKUpdataEnd;
            }
        }
        else
        {
            result = 5;
            goto HZKUpdataEnd;
        }
        
        f_close(FontFIL);
    }

    /* store HZK font info */
    myMemcpy(HzkFontInfo.Font, "hzk ", 4);
    ret = QFL_WriteNoChk((uint8_t *)&HzkFontInfo, HZKFONT_ADDR, sizeof(HzkFontInfo_t));
    if(ret != 0)
    {
        result = 7;
        goto HZKUpdataEnd;
    }

HZKUpdataEnd:    
    myFree(MLCSRC_AXISRAM, FontFIL);
    
    return (result);
}


