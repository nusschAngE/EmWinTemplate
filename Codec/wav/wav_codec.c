
#include "my_malloc.h"

#include "wav_codec.h"
#include "my_malloc.h"
//#include "memory.h"

/* cache buffer */
uint8_t WavCache[WAV_CACHESIZE];

uint16_t WavCacheOfs = 0;//cache offset
uint16_t WavDataOfs = 0;//data buffer offset
uint32_t WavFileSize = 0;//file byte offset
uint32_t WavDataSize = 0;//file data chunk size
uint32_t WavAudOfs = 0;//audio data byte offset
uint16_t WavStreamOfs = 0;//file stream byte offset
uint8_t  WavChunkIdx = 0;//file data chunk index


/**************** PUBLIC FUNCTION ******************/
uint8_t Wav_CodecInit(void)
{
    myMemset(WavCache, 0, WAV_CACHESIZE);
    /* init */
    WavDataSize = 0;
    WavAudOfs = 0;
    WavStreamOfs = 0;
    WavChunkIdx = 0;
    WavCacheOfs = 0;
    WavDataOfs = 0; 
    
    return (0);
}

void Wav_CodecDeInit(void)
{
    myFree(MLCSRC_AXISRAM, WavCache);
    WavDataSize = 0;
    WavAudOfs = 0;
    WavStreamOfs = 0;
    WavChunkIdx = 0;
    WavCacheOfs = 0;
    WavDataOfs = 0;
}

uint8_t Wav_FileOpen(WAVFIL *file, const uint8_t* path, uint8_t mode)
{
    uint8_t ret = 0;

    ret = f_open(file, (const TCHAR*)path, mode);
    /* open error */
    if(ret != FR_OK)    return (1);
    /* open success */
    return (0);
}

uint8_t Wav_FileRead(WAVFIL *file, void *buff, uint32_t btr, uint32_t *br)
{
    uint8_t ret = 0;

    ret = f_read(file, buff, btr, br);
    /* read error */
    if(ret != FR_OK)    return (1);
    /* read success */
    return (0);
}

uint8_t Wav_FileWrite(WAVFIL *file, const void *buff, uint32_t btw, uint32_t *bw)
{
    uint8_t ret = 0;

    ret = f_write(file, buff, btw, bw);
    /* write error */
    if(ret != FR_OK)    return (1);
    /* write success */
    return (0);
}

uint8_t Wav_FileClose(WAVFIL *file)
{
    uint8_t ret = 0;

    ret = f_close(file);
    /* close error */
    if(ret != FR_OK)    return (1);
    /* close success */
    return (0);
}

