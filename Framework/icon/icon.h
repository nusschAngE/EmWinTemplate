
#ifndef _ICON_H
#define _ICON_H

#include "public.h"


/* ICON List */
typedef enum
{
    ICON_DEFAULT = 0,
    /* APP entry icon */
    ICON_APP_MB_SELECTED,
    ICON_APP_MB_UNSELECTED,
    /* menu icon */
    ICON_MENU_TILTLE,
    /*  */
    ICON_MENU_ITEM,
    /* */
    ICON_FILE_MP3,
    ICON_FILE_WAV,
    ICON_FILE_NES,
    
    ICON_SRC_MAX
}ICON_SRC;

typedef struct
{
    uint32_t addr;//address in flash
    uint32_t size;//number bytes of icon source
    uint16_t w;
    uint16_t h;
}IconSource_t;







/**** PUBLIC FUNCTION ****/
extern void Icon_DrawScreen(uint16_t x, uint16_t y, uint16_t icon);

extern void Icon_DrawGRAM(uint16_t x, uint16_t y, uint16_t icon);


#endif


