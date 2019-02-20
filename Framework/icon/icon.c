
#include "icon.h"
#include "icon_source.h"

#include "tft_lcd.h"
#include "qspi_flash.h"

/* QSPI Read */
#define READ_ICON(cycle, addr, rsize)\
        QFL_ReadWithCycle(cycle, addr, rsize)

/*  */
static uint8_t cycleByte = 0;
static uint16_t lcdData = 0;

static void lcdCycle(uint8_t data)
{
    /* receive one byte */
    ++cycleByte;
    /* get lcd color data */
    if(cycleByte == 1)
    {
        lcdData = data;
    }
    else//two bytes
    {
        cycleByte = 0;
        lcdData |= (uint16_t)data << 8;
        /* write to LCD */
        LCD->LCD_RAM = lcdData;
    }    
}

static void ramCycle(uint8_t data)
{
    
}

/* draw into LCD-GRAM */
void Icon_DrawScreen(uint16_t x, uint16_t y, uint16_t icon)
{

}

/* draw into RAM-GRAM */
void Icon_DrawGRAM(uint16_t x, uint16_t y, uint16_t icon)
{
        
}
