
#ifndef _TFT_LCD_H
#define _TFT_LCD_H

#include "public.h"

/* Tx-Cplt call back */
typedef void(*XsferCplt)(void);

/* LCD default size */
#define DEFAULT_LCD_WIDTH       ((uint16_t)480)
#define DEFAULT_LCD_HEIGHT      ((uint16_t)800)

#define COLOR_BLACK             ((uint16_t)0x0000)
#define COLOR_WHITE             ((uint16_t)0xFFFF)

/* LCD fmc */
typedef struct
{
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
}LCD_t;

/* FMC_A18 -> LCD_RS 
*  FMC_NE1 -> LCD_CS 
*/
#define LCD_BASE    ((uint32_t)0x60000000 | 0x0007fffe)
#define LCD         ((LCD_t*)LCD_BASE)

typedef struct
{
    uint32_t id;
    uint8_t  dir;
    uint16_t width;
    uint16_t height;
}LCDDevice_t;

/* LCD device struct */
extern LCDDevice_t lcdDev;


/** public func **/
uint8_t TFTLCD_Init(void);
void TFTLCD_SetDispOnOff(bool on);
void TFTLCD_SetBkliteOnOff(bool on);
void TFTLCD_SetScanDirection(uint8_t dirt);
void TFTLCD_Clear(uint16_t color);
void TFTLCD_SetWorkRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void TFTLCD_StartDataSession(void);
void TFTLCD_FlushData(uint16_t *pData, uint32_t size);
void TFTLCD_FillBlock(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void TFTLCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void TFTLCD_FastDrawPoint(uint16_t x, uint16_t y, uint16_t color);
uint16_t TFTLCD_ReadPoint(uint16_t x, uint16_t y);
void TFTLCD_FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void TFTLCD_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, int field);
void TFTLCD_FillBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *bmp);

/* LCD DMA Functions */
uint8_t TFTLCD_DMAChannelInit(XsferCplt xsferCpltFunc);
void TFTLCD_DMAXsferStart(uint16_t *pData, uint32_t size);
void TFTLCD_DMAXsferStop(void);

#endif

