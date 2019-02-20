/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUICharP.C
Purpose     : Implementation of Proportional fonts
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"
#include "qspi_flash.h"
#include "lcd.h"
#include "mytext.h"
#include "string.h"
#include "EmWinHZFont.h"
#include "EmWin_ASCII_Font.h"

uint8_t GetAscFontBin(uint8_t *bin, uint8_t fontSize, uint8_t ch)
{
    uint8_t binSize = 0, i;
    uint32_t ofs = 0;

    /* unsupport font size */
    if(fontSize >= FONT_SIZE_UN) {
        return (0);
    }

    binSize = ASCFont[fontSize].binSize;
    ofs = ch - ' ';
    
    if(ofs >= NUMS_OF_ASCII) {
        ofs = 0;
    }

    switch (fontSize)
    {
        case FONT_SIZE_12:
            for(i = 0; i < binSize; i++) {
                bin[i] = emwin_asc2_1206[ofs][i];
            }
            break;
        case FONT_SIZE_16: 
            for(i = 0; i < binSize; i++) {
                bin[i] = emwin_asc2_1608[ofs][i];
            }
            break;
        case FONT_SIZE_24: 
            for(i = 0; i < binSize; i++) {
                bin[i] = emwin_asc2_2412[ofs][i];
            }
            break;
        case FONT_SIZE_32: 
            for(i = 0; i < binSize; i++) {
                bin[i] = emwin_asc2_3216[ofs][i];
            }
            break;
    }

    return (binSize);
}

uint8_t GetHzkFontBin(uint8_t *bin, uint8_t fontSize, uint16_t gbk)
{
    uint32_t ofs = 0;
    uint8_t binSize = 0;
    uint8_t hVal, lVal;

    /* unsupport font size */
    if(fontSize >= FONT_SIZE_UN) {
        return (0);
    }

    binSize = HZKFont[fontSize].binSize;

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
    ofs = (190 * hVal + lVal) * binSize; //font data offset
    //printf("gbk = %d, %d - %d - %d, flash addr = 0x%08x\r\n", gbk, hVal, lVal, ofs, ofs + HzkFontInfo.hzk16Addr);
    /* get font bin */
    switch (fontSize)
    {
        case FONT_SIZE_12:
            QFL_Read(bin, ofs+HzkFontInfo.hzk12Addr, binSize);
            break;
        case FONT_SIZE_16:
            QFL_Read(bin, ofs+HzkFontInfo.hzk16Addr, binSize);
            break;
        case FONT_SIZE_24:
            QFL_Read(bin, ofs+HzkFontInfo.hzk24Addr, binSize);
            break;
        case FONT_SIZE_32:
            QFL_Read(bin, ofs+HzkFontInfo.hzk32Addr, binSize);
            break;
    }    

    return (binSize);
}

//Get Font bin data
static void GUI_GetDataFromMemory(const GUI_FONT_PROP GUI_UNI_PTR *pProp, U16P c)
{
    unsigned char fontsize;
    U16 BytesPerFont;
	GUI_FONT EMWINFONT;
	
	EMWINFONT = *GUI_pContext->pAFont;
    BytesPerFont = GUI_pContext->pAFont->YSize * pProp->paCharInfo->BytesPerLine;
    if (BytesPerFont > FONT_BIN_MSIZE) {
        BytesPerFont = FONT_BIN_MSIZE;
    }
	    
	//Get Font Size
	if(memcmp(&EMWINFONT, &GUI_FontHZ12, sizeof(GUI_FONT)) == 0) {
	    fontsize = FONT_SIZE_12;
    } else if(memcmp(&EMWINFONT, &GUI_FontHZ16, sizeof(GUI_FONT)) == 0) {
        fontsize = FONT_SIZE_16;
    } else if(memcmp(&EMWINFONT, &GUI_FontHZ24, sizeof(GUI_FONT)) == 0) {
        fontsize = FONT_SIZE_24; 
    } else if(memcmp(&EMWINFONT, &GUI_FontHZ32, sizeof(GUI_FONT)) == 0) {
        fontsize = FONT_SIZE_32;
    }
    
    memset(FontBIN, 0, FONT_BIN_MSIZE);   
    if (c < 0x80) {//ASCII 
        GetAscFontBin(FontBIN, fontsize, (uint8_t)c);         
	} else { //GBK
        GetHzkFontBin(FontBIN, fontsize, (uint16_t)c);
	}   	
}
/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_X_DispChar(U16P c) 
{	
	int BytesPerLine;
    GUI_DRAWMODE DrawMode = GUI_pContext->TextMode;
    const GUI_FONT_PROP GUI_UNI_PTR *pProp = GUI_pContext->pAFont->p.pProp;
    //
    for (; pProp; pProp = pProp->pNext) 
    {
        if ((c >= pProp->First) && (c <= pProp->Last)) break;
    }
    
    if (pProp)
    {
        GUI_DRAWMODE OldDrawMode;
        const GUI_CHARINFO GUI_UNI_PTR * pCharInfo = pProp->paCharInfo;
        
        GUI_GetDataFromMemory(pProp, c);
        BytesPerLine = pCharInfo->BytesPerLine;                
        OldDrawMode  = LCD_SetDrawMode(DrawMode);
        LCD_DrawBitmap(GUI_pContext->DispPosX, GUI_pContext->DispPosY,
                       pCharInfo->XSize, GUI_pContext->pAFont->YSize,
                       GUI_pContext->pAFont->XMag, GUI_pContext->pAFont->YMag,
                       1,     /* Bits per Pixel */
                       BytesPerLine,
                       &FontBIN[0],
                       &LCD_BKCOLORINDEX
                       );
        /* Fill empty pixel lines */
        if (GUI_pContext->pAFont->YDist > GUI_pContext->pAFont->YSize) 
        {
            int YMag = GUI_pContext->pAFont->YMag;
            int YDist = GUI_pContext->pAFont->YDist * YMag;
            int YSize = GUI_pContext->pAFont->YSize * YMag;
            if (DrawMode != LCD_DRAWMODE_TRANS) 
            {
                LCD_COLOR OldColor = GUI_GetColor();
                GUI_SetColor(GUI_GetBkColor());
                LCD_FillRect(GUI_pContext->DispPosX, GUI_pContext->DispPosY + YSize, 
                             GUI_pContext->DispPosX + pCharInfo->XSize, 
                             GUI_pContext->DispPosY + YDist);
                GUI_SetColor(OldColor);
            }
        }
        LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
		GUI_pContext->DispPosX += pCharInfo->XDist * GUI_pContext->pAFont->XMag;
    }
}

/*********************************************************************
*
*       GUIPROP_GetCharDistX
*/
int GUIPROP_X_GetCharDistX(U16P c) 
{
    const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUI_pContext->pAFont->p.pProp;  
    for (; pProp; pProp = pProp->pNext)                                         
    {
        if ((c >= pProp->First) && (c <= pProp->Last))break;
    }
    return (pProp) ? (pProp->paCharInfo)->XSize * GUI_pContext->pAFont->XMag : 0;
}
