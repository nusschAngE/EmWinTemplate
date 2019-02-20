
#include "qspi_flash.h"
#include "tft_lcd.h"

#include "font.h"
#include "ascii.h"
#include "mytext.h"
#include "my_malloc.h"
//#include "ff.h"

const ASCFont_t ASCFont[FONT_SIZE_UN] = 
{
    {FONT_SIZE_12,  12,  6, 12},
    {FONT_SIZE_16,  16,  8, 16},
    {FONT_SIZE_24,  36, 12, 24},
    {FONT_SIZE_32,  64, 16, 32}
};

const HZKFont_t HZKFont[FONT_SIZE_UN] = 
{
    {FONT_SIZE_12,  24, 12, 12},
    {FONT_SIZE_16,  32, 16, 16},
    {FONT_SIZE_24,  72, 24, 24},
    {FONT_SIZE_32, 128, 32, 32}
};

/*  STATIC
*/
static uint16_t Fill_FontGRAM_ASCII(uint16_t *ram, uint8_t ch, uint8_t size, uint16_t fg, uint16_t bg);
static uint16_t Fill_FontGRAM_GBK(uint16_t *ram, uint16_t gbk, uint8_t size, uint16_t fg, uint16_t bg);
static void showASCII(uint16_t x, uint16_t y, uint8_t ch, FONT_t font);
static void showGBK(uint16_t x, uint16_t y, uint16_t gbk, FONT_t font);

/* font bmp data */
uint8_t FontBIN[FONT_BIN_MSIZE];
/* font picture */
//#define FONT_GRAM_SIZE  (32 * 32)
uint16_t FontGRAM[FONT_GRAM_SIZE];

/**************** PUBLIC FUNCTION *******************/

void ShowTextLineGbk(uint16_t x, uint16_t y, const uint8_t *pText, FONT_t font)
{
    uint16_t i = 0;
    uint16_t x0 = x, y0 = y;
    uint16_t code = 0;

    /* check work region */
    if(x >= lcdDev.width || y >= lcdDev.height)
    {
        return ;
    }

    for(i = 0; pText[i] != 0; )
    {
        code = (uint16_t)pText[i++];//lsb
        if(code < 0x80)//ascii
        {
            /* display */
            showASCII(x0, y0, (uint8_t)code, font);        
            /* posit */
            x0 += ASCFont[font.size].width;
        }
        else
        {
            code |= (uint16_t)pText[i++] << 8;//msb
            /* display */
            showGBK(x0, y0, code, font);        
            /* posit */
            x0 += HZKFont[font.size].width;
        }
        /* check work region */
        if(x0 >= lcdDev.width)
        {
            break;
        }
    }
}

void ShowTextLineUnicode(uint16_t x, uint16_t y, const uint8_t *pText, FONT_t font)
{
#if 0
    uint16_t x0 = x, y0 = y;
    uint32_t bWrite = 0, i = 0;
    uint16_t code = 0, uni = 0;

    if(x >= lcdDev.width || y >= lcdDev.height)
    {
        return ;
    }

    for(i = 0; pText[i] || pText[i+1]; )
    {
        uni = (uint16_t)pText[i++];//lsb
        uni |= (uint16_t)pText[i++] << 8;//msb
        /* unicode --> gbk */
        code = ff_uni2oem((DWORD)uni, FF_CODE_PAGE);
        if(code < 0x80)//ascii
        {
            /* display */
            showASCII(x0, y0, (uint8_t)code, font);        
            /* posit */
            x0 += ASCFont[font.size].width;
        }
        else
        {
            /* display */
            showGBK(x0, y0, code, font);        
            /* posit */
            x0 += HZKFont[font.size].width;
        }
        /* check work region */
        if(x0 >= lcdDev.width)
        {
            break;
        }
    }
#endif
}

void ShowTextLineAscii(uint16_t x, uint16_t y, const char *pText, FONT_t font)
{
    uint16_t i = 0;
    uint16_t x0 = x, y0 = y;

    if(x >= lcdDev.width || y >= lcdDev.height)
    {
        return ;
    }

    for(i = 0; pText[i] != '\0'; i++)
    {
        /* display */
        showASCII(x0, y0, pText[i], font);        
        /* posit */
        x0 += ASCFont[font.size].width;
        /* check work region */
        if(x0 >= lcdDev.width)
        {
            break;
        }
    }
}

uint16_t TextDispWidth(uint8_t *pText, uint8_t fontSize)
{
    uint16_t width = 0, i = 0, code = 0;

    if(pText == NULL) {return 0;}

    for(i = 0; pText[i] != 0; )
    {
        code = pText[i++];//lsb
        if(code < 0x80)//ascii
        {
            width += ASCFont[fontSize].width;
        }
        else
        {
            code |= (uint16_t)pText[i++] << 8;//msb
            /* posit */
            width += HZKFont[fontSize].width;
        }
        /* check work region */
        if(width >= lcdDev.width)
        {
            width = lcdDev.width;
            break;
        }
    }

    return (width);
}

uint16_t TextDispHeight(uint8_t *pText, uint8_t fontSize)
{
    (void)pText;

    return (ASCFont[fontSize].height);
}

/******************* private function **********************/

/* 
*   @param size : font size
*/
static uint16_t Fill_FontGRAM_ASCII(uint16_t *ram, uint8_t ch, uint8_t size, uint16_t fg, uint16_t bg)
{
    uint8_t ofs;
    uint16_t i,j;

    uint8_t x = 0, y = 0, temp = 0;
    uint8_t width, height;
    uint16_t FontBmpSize, FontBinSize;

    myMemset(FontBIN, 0, FONT_BIN_MSIZE);

    /* unsupport font size */
    if(size >= FONT_SIZE_UN)
    {
        return (0);
    }
    
    width = ASCFont[size].width;
    height = ASCFont[size].height;
    FontBinSize = ASCFont[size].binSize;
    FontBmpSize = ASCFont[size].width * ASCFont[size].height;

	//printf("%d - %d - %d - %d - %d\r\n", size, width, height, FontBinSize, FontBmpSize);
    /* get 'ch' offset */
    ofs = ch - ' ';
    if(ofs >= NUMS_OF_ASCII) {
        ofs = 0;
    }
    /* get font bin */
    for(i = 0; i < FontBinSize; i++)
    {
        switch (size)
        {
            case FONT_SIZE_12: FontBIN[i] = asc2_1206[ofs][i]; break;
            case FONT_SIZE_16: FontBIN[i] = asc2_1608[ofs][i]; break;
            case FONT_SIZE_24: FontBIN[i] = asc2_2412[ofs][i]; break;
            case FONT_SIZE_32: FontBIN[i] = asc2_3216[ofs][i]; break;
        }
    }
    /* fill font bmp */
    for(i = 0; i < FontBinSize; i++)
    {
        temp = FontBIN[i];
        for(j = 0; j < 8; j++)
        {
            /* set dot */
            if(temp & 0x80) ram[x + (y * width)] = fg;
            else            ram[x + (y * width)] = bg;
            /* next dot */
            temp <<= 1;
            y++;
            if(y >= height)
            {
                y = 0;
                x++;
                break;
            }            
        }
    }
    
    return FontBmpSize;
}

/* 
*   @param size : font size
*/
static uint16_t Fill_FontGRAM_GBK(uint16_t *ram, uint16_t gbk, uint8_t size, uint16_t fg, uint16_t bg)
{
    uint32_t ofs;
    uint16_t i,j;
    uint8_t hVal, lVal;
    uint8_t x = 0, y = 0, temp = 0;
    uint8_t width, height;
    uint16_t FontBmpSize, FontBinSize;

    myMemset(FontBIN, 0, FONT_BIN_MSIZE);    

    /* unsupport font size */
    if(size >= FONT_SIZE_UN)
    {
        return (0);
    }

    width = HZKFont[size].width;
    height = HZKFont[size].height;
    FontBinSize = HZKFont[size].binSize;
    FontBmpSize = HZKFont[size].width * HZKFont[size].height;

    //hVal = (uint8_t)(gbk >> 8);
    //lVal = (uint8_t)(gbk & 0x00ff);
    hVal = (uint8_t)(gbk & 0x00ff);
    lVal = (uint8_t)(gbk >> 8);
    /* invalid code */
    if((hVal < 0x81) || (lVal < 0x40) || (hVal == 0xff) || (lVal== 0xff))
    {
        return (0);
    }
    /* special code process */
	if(lVal < 0x7f) lVal -= 0x40;
	else            lVal -= 0x41;
    /*  */
    hVal -= 0x81;
    //ofs = ((lVal - 0xA1)*94 + (hVal - 0xA1)) * FontBinSize;// font data offset
    ofs = (190 * hVal + lVal) * FontBinSize; //font data offset
    //printf("gbk = %d, %d - %d - %d, flash addr = 0x%08x\r\n", gbk, hVal, lVal, ofs, ofs + HzkFontInfo.hzk16Addr);
    /* get font bin */
    switch (size)
    {
        case FONT_SIZE_12:
            QFL_Read(FontBIN, ofs + HzkFontInfo.hzk12Addr, FontBinSize);
            break;
        case FONT_SIZE_16:
            QFL_Read(FontBIN, ofs + HzkFontInfo.hzk16Addr, FontBinSize);
            break;
        case FONT_SIZE_24:
            QFL_Read(FontBIN, ofs + HzkFontInfo.hzk24Addr, FontBinSize);
            break;
        case FONT_SIZE_32:
            QFL_Read(FontBIN, ofs + HzkFontInfo.hzk32Addr, FontBinSize);
            break;
    }
    
    /* fill font bmp */
    for(i = 0; i < FontBinSize; i++)
    {
        temp = FontBIN[i];
        for(j = 0; j < 8; j++)
        {
            /* set dot */
            if(temp & 0x80) ram[x + (y * width)] = fg;
            else            ram[x + (y * width)] = bg;
            /* next dot */
            temp <<= 1;
            y++;
            if(y >= height)
            {
                y = 0;
                x++;
                break;
            }            
        }
    }
                		
	return FontBmpSize;
}

static void showASCII(uint16_t x, uint16_t y, uint8_t ch, FONT_t font)
{
    uint16_t width, height;

    myMemset(FontGRAM, 0, FONT_GRAM_SIZE);
    /* get font bmp */
    Fill_FontGRAM_ASCII(FontGRAM, ch, font.size, font.fg, font.bg);
    /* get font size */
    width = ASCFont[font.size].width;
    height = ASCFont[font.size].height;
    /* display font bmp */
    TFTLCD_FillBitmap(x, y, width, height, FontGRAM);
}

static void showGBK(uint16_t x, uint16_t y, uint16_t gbk, FONT_t font)
{
    uint16_t width, height;

    myMemset(FontGRAM, 0, FONT_GRAM_SIZE);
    /* get font bmp */
		Fill_FontGRAM_GBK(FontGRAM, gbk, font.size, font.fg, font.bg);
    /* get font size */
    width = HZKFont[font.size].width;
    height = HZKFont[font.size].height;
    /* display font bmp */
    TFTLCD_FillBitmap(x, y, width, height, FontGRAM);
}


