
#ifndef _WAV_CODEC_H
#define _WAV_CODEC_H

#include "public.h"
#include "fatfs_ex.h"
#include <stdio.h>

/* file system */
typedef FIL     WAVFIL;

typedef struct
{
    uint16_t IBitsPerSample;
    uint32_t ChannelMask;
    uint16_t EncodeFmt;
    void    *Reserve;
}WavExpand_t;

typedef struct
{
    uint16_t EncodeFmt;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    //EXPAND
    uint16_t    ExpandSize;
    WavExpand_t Expand;
}WavFmt_t;

typedef struct
{
    uint32_t BytesPerSample;
}WavFact_t;

typedef struct
{
    //RIFF
    uint8_t  RiffID[4];
    uint32_t FileSize;
    uint8_t  FileFmt[4];
    //FMT
    uint8_t  FmtID[4];
    uint32_t FmtSize;
    WavFmt_t Fmt;  
    //FACT
    uint8_t   FactID[4];
    uint32_t  FactSize; 
    WavFact_t Fact;  
    //DATA
    uint8_t  DataID[4];
    uint32_t AudioDataSize;
}WavHeader_t;

/* wav data chunk
*/
enum
{
    WAV_CHUNK_RIFFID = 0,
    WAV_CHUNK_FILESIZE,
    WAV_CHUNK_FILEFMT,

    WAV_CHUNK_FMTID,
    WAV_CHUNK_FMTSIZE,
    WAV_CHUNK_FMT,

    WAV_CHUNK_FACTID,
    WAV_CHUNK_FACTSIZE,
    WAV_CHUNK_FACT,

    WAV_CHUNK_DATAID,
    WAV_CHUNK_DATASIZE,
    WAV_CHUNK_DATA,
    

    WAV_CHUNK_END,
};

/* fmt type
*/
enum
{
    WAV_FMT_PCM         = 0x0001,
    WAV_FMT_MADPCM      = 0x0002,
    WAV_FMT_IEEEF       = 0x0003,
    WAV_FMT_ITUGaLOW    = 0x0006,
    WAV_FMT_ITUGuLOW    = 0x0007,
    WAV_FMT_GSM         = 0x0031,
    WAV_FMT_ITUGADPCM   = 0x0040,
    
    WAV_FMT_OTHER       = 0xfffe,
};

/* wav decode error
*/
enum
{
    WAV_DECODE_ERROR_NONE = 0,
    WAV_DECODE_ERROR_PARAM,
    WAV_DECODE_ERROR_BUFFER,
    WAV_DECODE_ERROR_RIFFID,
    WAV_DECODE_ERROR_FMTID,
    WAV_DECODE_ERROR_FMT,
    WAV_DECODE_ERROR_FACTID,
    WAV_DECODE_ERROR_FACT,
    WAV_DECODE_ERROR_DATAID,
    WAV_DECODE_ERROR_DATA,
    WAV_DECODE_ERROR_FRAME,
};

#define WAV_CACHESIZE     (256U)
/* cache buffer */
extern uint8_t WavCache[WAV_CACHESIZE];

extern uint16_t WavCacheOfs;//cache offset
extern uint16_t WavDataOfs;//data buffer offset
extern uint32_t WavFileSize;//file byte offset
extern uint32_t WavDataSize;//file data chunk size
extern uint32_t WavAudOfs;//audio data offset
extern uint16_t WavStreamOfs;//file stream byte offset
extern uint8_t  WavChunkIdx;//file data chunk index

/**************** PUBLIC FUNCTION ******************/
extern uint8_t Wav_CodecInit(void);
extern void Wav_CodecDeInit(void);
extern uint8_t Wav_FileOpen(WAVFIL *file, const uint8_t* path,uint8_t mode);
extern uint8_t Wav_FileRead(WAVFIL *file, void *buff, uint32_t btr, uint32_t *br);
extern uint8_t Wav_FileWrite(WAVFIL *file, const void *buff, uint32_t btw, uint32_t *bw);
extern uint8_t Wav_FileClose(WAVFIL *file);
/*  decode function
*/
extern uint8_t Wav_DecodeInit(uint32_t FileSize);
extern uint8_t Wav_GetHeader(WavHeader_t *header, uint8_t *pData, uint32_t pSize, uint32_t *bDecode);
extern uint8_t Wav_AudioDataProc(WavHeader_t *header, uint8_t *pBuff, uint32_t buffSize, const uint8_t *pData, uint32_t dataSize, uint32_t *bFill);

#endif

