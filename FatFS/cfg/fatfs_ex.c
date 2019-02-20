
#include "font.h"

#include "fatfs_ex.h"

/*  */
FATFS SDFatFS;
FATFS USBFatFS;
FATFS NANDFatFS;
FATFS **tmpFS;

bool IsSDMount = FALSE;
bool IsUSBMount = FALSE;
bool IsNANDMount = FALSE;

/*  file extension 
*  
*   match to enum-'FTYPE'
*/
const char *FileExt[] = 
{
    ".BIN",
    ".FON",
    ".TXT",
    ".WAV",
    ".MP3",

    NULL
};

/* FATFS base driver 
*  if SD is not mount, system will not run
*/
FRESULT SDCard_Mount(void)
{
    FRESULT fret = FR_OK;

    fret = f_mount(&SDFatFS, SdFsDrv, 1);
    if(fret == FR_OK) {
        IsSDMount = TRUE;
    } else {
        IsSDMount = FALSE;
    }

    return (fret);
}

FTYPE f_TypeTell(TCHAR* fname)
{
    uint8_t tmp[16];
    int16_t i = 0, j = -1;

    /* find extension */
    while(fname[i] != 0x00)
    {
        if(fname[i] == '.') j = i;//mark '.'
                    
        i++;
    }
    /* not find */
    if(j < 0) {
        return (T_UNSUPRT);
    }
    /* get extension */
    for(i = 0; (fname[j]!=0)&&(i<16); j++) {
        tmp[i++] = char_upper(fname[j]);
    }
    /* match extension */
    for(j = 0; j < T_UNSUPRT; j++)
    {
        if(myMemcmp((const uint8_t*)tmp, FileExt[j], i) == 0) {
            return ((FTYPE)j);
        }
    }

    return (T_UNSUPRT);
}

/* the size of path must big enough */
uint16_t f_GetFilePath(TCHAR *path, uint16_t pSize, TCHAR *dirName, TCHAR *fileName)
{
    uint16_t len = 0;

    if(!path) {
        return (0);
    }

    myMemset(path, 0, pSize);
    myStrcat((char*)path, (const char *)dirName);
    myStrcat((char*)path, "/");
    myStrcat((char*)path, (const char *)fileName);
    len = myStrlen((const char *)path);

    return (len);
}

/* the size of dirName must big enough */
uint16_t f_GetDirName(TCHAR *path, uint16_t pSize, TCHAR *dirName)
{
    uint16_t len = 0, i = 0;

    if(!path || !dirName || !pSize) {
        return (0);
    }

    for(i = 0; i < pSize; i++)
    {
        if(path[i] == ':') {len = i;}//mark ':'
        if(path[i] == '/') {len = i;}//mark '/'
        if(path[i] == '\0') {break;}//path end
        if(i >= pSize) {return (0);}//over length
    }
    len++;//skip mark char
    myMemcpy(dirName, (const uint8_t*)path, len);

    return (len);
}

/* the size of fileName must big enough */
uint16_t f_GetFileName(TCHAR *path, uint16_t pSize, TCHAR *fileName)
{
    uint16_t len = 0, i = 0, idx = 0;

    if(!path || !fileName || !pSize) {
        return (0);
    }

    for(i = 0; i < pSize; i++)
    {
        if(path[i] == ':') {idx = i;}//mark ':'
        if(path[i] == '/') {idx = i;}//mark '/'
        if(path[i] == '\0') {break;}//path end
        if(i >= pSize) {return (0);}//over length
    }
    idx++;//skip mark char
    len = pSize - idx;
    myMemcpy(fileName, (const uint8_t*)path+idx, len);

    return (len);
}

#if defined(ExtFLASH_CODE_PAGE)
#include "qspi_flash.h"
#include "font.h"

#define FF_ExtFlashRead(code, addr)   QFL_Read((uint8_t*)&code, addr, 4)

WCHAR ff_uni2oem (	/* Returns OEM code character, zero on error */
	DWORD	uni,	/* UTF-16 encoded character to be converted */
	WORD	cp		/* Code page for the conversion */
)
{
	WCHAR c = 0, uc, tmp[2];
	UINT i = 0, n, li, hi;
	UINT addr = 0, pageSize = 0;

	if (uni < 0x80) 
	{	/* ASCII? */
		c = (WCHAR)uni;
	}
	else
	{   /* Non-ASCII */
		if (uni < 0x10000 && cp == FF_CODE_PAGE) 
		{	/* Is it in BMP and valid code page? */
		    addr = HzkFontInfo.unigbkAddr;
		    pageSize = HzkFontInfo.unigbkSize/2;
			uc = (WCHAR)uni;
			hi = pageSize/4 - 1;
			li = 0;
			for (n = 16; n; n--) 
			{
				i = li + (hi - li) / 2;
				FF_ExtFlashRead(tmp, addr+i*4);
				if (uc == tmp[0]) {break;}
				
				if (uc > tmp[0]) {li = i;}
				else {hi = i;}
			}
			c = n ? tmp[1] : 0;
		}
	}

	return c;
}

WCHAR ff_oem2uni (	/* Returns Unicode character, zero on error */
	WCHAR	oem,	/* OEM code to be converted */
	WORD	cp		/* Code page for the conversion */
)
{
	WCHAR c = 0, tmp[2];
	UINT i = 0, n, li, hi;
    UINT addr = 0, pageSize = 0;

	if (oem < 0x80) 
	{	/* ASCII? */
		c = oem;
	} 
	else 
	{	/* Extended char */
		if (cp == FF_CODE_PAGE) 
		{	/* Is it valid code page? */
			addr = HzkFontInfo.unigbkAddr + HzkFontInfo.unigbkSize/2;
		    pageSize = HzkFontInfo.unigbkSize/2;
			hi = pageSize/4 - 1; //size
			li = 0;
			for (n = 16; n; n--) 
			{
				i = li + (hi - li) / 2;
				FF_ExtFlashRead(tmp, addr+i*4);
				if (oem == tmp[0]) {break;}
				
				if (oem > tmp[0]) {li = i;}
				else {hi = i;}
			}
			c = n ? tmp[1] : 0;
		}
	}

	return c;
}

#endif //ExtFLASH_CODE_PAGE


