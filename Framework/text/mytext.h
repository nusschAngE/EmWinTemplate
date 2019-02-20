
#ifndef _MY_TEXT_H
#define _MY_TEXT_H

#include "public.h"
#include "font.h"

/* font picture */
#define FONT_BIN_MSIZE  (32 * 32 / 8)
#define FONT_GRAM_SIZE  (32 * 32)
extern uint8_t FontBIN[FONT_BIN_MSIZE];;
extern uint16_t FontGRAM[FONT_GRAM_SIZE];

/*  ASCII Font
**/
typedef struct
{
    uint8_t  size;
    uint16_t binSize;
    uint16_t width;
    uint16_t height;
}ASCFont_t;

/*  HZK Font
**/
typedef struct
{
    uint8_t  size;
    uint16_t binSize;
    uint16_t width;
    uint16_t height;
}HZKFont_t;

extern const ASCFont_t ASCFont[FONT_SIZE_UN];
extern const HZKFont_t HZKFont[FONT_SIZE_UN];

/* PUBLIC FUNC
*/
extern void ShowTextLineGbk(uint16_t x, uint16_t y, const uint8_t *pText, FONT_t font);
extern void ShowTextLineUnicode(uint16_t x, uint16_t y, const uint8_t *pText, FONT_t font);
extern void ShowTextLineAscii(uint16_t x, uint16_t y, const char *pText, FONT_t font);
extern uint16_t TextDispWidth(uint8_t *pText, uint8_t fontSize);
extern uint16_t TextDispHeight(uint8_t *pText, uint8_t fontSize);
#endif

