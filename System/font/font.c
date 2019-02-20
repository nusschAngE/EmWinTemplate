
#include "stm32h7xx.h"

#include "font.h"
#include "qspi_flash.h"
#include "my_malloc.h"

/* Font Preset
*/
static const FONT_t FontPresetTab[] =  
{
    {FONT_SIZE_12, FONT_FG_WHITE, FONT_BG_BLACK},
    {FONT_SIZE_16, FONT_FG_WHITE, FONT_BG_BLACK},
    {FONT_SIZE_24, FONT_FG_WHITE, FONT_BG_BLACK},
    {FONT_SIZE_32, FONT_FG_WHITE, FONT_BG_BLACK},

    {FONT_SIZE_12, FONT_FG_BLACK, FONT_BG_WHITE},
    {FONT_SIZE_16, FONT_FG_BLACK, FONT_BG_WHITE},
    {FONT_SIZE_24, FONT_FG_BLACK, FONT_BG_WHITE},
    {FONT_SIZE_32, FONT_FG_BLACK, FONT_BG_WHITE},
};

/* font in flash */
HzkFontInfo_t HzkFontInfo;

/* check HZK Font Info */
uint8_t HZKFont_Check(void)
{
    uint8_t ret = 0;
	
    ret = QFL_Read((uint8_t *)&HzkFontInfo, HZKFONT_ADDR, sizeof(HzkFontInfo_t));
    if(ret == 0)
    {
        if(myMemcmp(HzkFontInfo.Font, "hzk ", 4) == 0) {
        	//printf("hzk font ready!\r\n");
    #if 0	
			printf("FontInfo->Font = %s\r\n", FontInfo->Font);
			printf("FontInfo->unigbkAddr = 0x%08x\r\n", FontInfo->unigbkAddr);
			printf("FontInfo->unigbkSize = 0x%08x\r\n", FontInfo->unigbkSize);
			printf("FontInfo->hzk12Addr = 0x%08x\r\n", FontInfo->hzk12Addr);
			printf("FontInfo->hzk12Size = 0x%08x\r\n", FontInfo->hzk12Size);
			printf("FontInfo->hzk16Addr = 0x%08x\r\n", FontInfo->hzk16Addr);
			printf("FontInfo->hzk16Size = 0x%08x\r\n", FontInfo->hzk16Size);
			printf("FontInfo->hzk24Addr = 0x%08x\r\n", FontInfo->hzk24Addr);
			printf("FontInfo->hzk24Size = 0x%08x\r\n", FontInfo->hzk24Size);
			printf("FontInfo->hzk32Addr = 0x%08x\r\n", FontInfo->hzk32Addr);
			printf("FontInfo->hzk32Size = 0x%08x\r\n", FontInfo->hzk32Size);            
    #endif
            return (0);
        } else {
            //printf("hzk font not exist!\r\n");
            return (1);
        }
    }
    else
    {
        //printf("hzk font check error, flash not ready!\r\ns");
        return (2);
    }
}

FONT_t GetFontPreset(uint8_t preset)
{
    assert_param(preset < (sizeof(FontPresetTab) / sizeof(FontPresetTab[0])));

    return FontPresetTab[preset];
}


