
#include "wav_codec.h"


/*  STATIC 
*/
static uint8_t wav_GetFmtContent(WavFmt_t *fmt, uint8_t *fmtBuff, uint32_t size);
static uint8_t wav_GetExpandContent(WavExpand_t *expand, uint8_t *expBuff, uint32_t size);
static uint8_t wav_GetFactContent(WavFact_t *fact, uint8_t *factBuff, uint32_t size);


/*************** PUBLIC FUNCTION ********************/

/*  @param audOfs : the offset of audio data in file 
*   @param pData  : ensure it contains all header info
*/
uint8_t Wav_GetHeader(WavHeader_t *header, uint8_t *pData, uint32_t pSize, uint32_t *bDecode)
{
    uint8_t ret = 0;
    uint8_t ChunkIdx = 0;
    uint8_t cidx = 0;
    uint32_t DataOfs = 0;

    *bDecode = 0;
    /* check param */
    if((header == NULL) || (pData == NULL))
    {
        return (WAV_DECODE_ERROR_PARAM);
    }
    /* decode */
    while(ChunkIdx <= WAV_CHUNK_DATA)
    {
        switch (ChunkIdx)
        {
            case WAV_CHUNK_RIFFID://"RIFF"
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* check "RIFF" */   
                    myMemcpy(header->RiffID, WavCache, 4);
                    //printf("header->RiffID : %s\r\n", header->RiffID);
                    if(myMemcmp(header->RiffID, "RIFF", 4) == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get 'RIFF' error\r\n");
                        return (WAV_DECODE_ERROR_RIFFID);
                    }
                }
                break;
            case WAV_CHUNK_FILESIZE:
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* get file szie */
                    header->FileSize =  (uint32_t)WavCache[3]<<24;
                    header->FileSize |= (uint32_t)WavCache[2]<<16;
                    header->FileSize |= (uint32_t)WavCache[1]<<8;
                    header->FileSize |= (uint32_t)WavCache[0];
                    ChunkIdx++;
                    if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                }
                break;
            case WAV_CHUNK_FILEFMT://"WAVE"
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* check "WAVE" */
                    myMemcpy(header->FileFmt, WavCache, 4);
                    //printf("header->FileFmt : %s\r\n", header->FileFmt);
                    if(myMemcmp(header->FileFmt, "WAVE", 4) == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get 'WAVE' error\r\n");
                        return (WAV_DECODE_ERROR_FMTID);
                    }
                }
                break;
            case WAV_CHUNK_FMTID://"fmt "
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* check "fmt " */   
                    myMemcpy(header->FmtID, WavCache, 4);
                    //printf("header->FmtID : %s\r\n", header->FmtID);
                    if(myMemcmp(header->FmtID, "fmt ", 4) == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get 'fmt ' error\r\n");
                        return (WAV_DECODE_ERROR_FMTID);
                    }
                }
                break;
            case WAV_CHUNK_FMTSIZE:
                {
    				for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
    				/* get fmt context size */
                    header->FmtSize =  (uint32_t)WavCache[3]<<24;
                    header->FmtSize |= (uint32_t)WavCache[2]<<16;
                    header->FmtSize |= (uint32_t)WavCache[1]<<8;
                    header->FmtSize |= (uint32_t)WavCache[0];
                    ChunkIdx++;
                    if(DataOfs+header->FmtSize > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                }
            	break;
            case WAV_CHUNK_FMT:
                {
                    for(cidx = 0; cidx < header->FmtSize; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* get fmt context */
                    ret = wav_GetFmtContent(&header->Fmt, WavCache, header->FmtSize);
                    if(ret == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get fmt content error, ret = %d\r\n", ret);
                        return (WAV_DECODE_ERROR_FMT);
                    }
                }
                break;
            
            case WAV_CHUNK_FACTID://"fact"
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* check 'fact' or 'data' */
                    myMemcpy(header->FactID, WavCache, 4);
                    //printf("header->FactID : %s\r\n", header->FactID);
                    if(myMemcmp(header->FactID, "fact", 4) == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else if(myMemcmp(header->FactID, "data", 4) == 0)
                    {
                        myMemset(header->FactID, 0, 4);
                        myMemcpy(header->DataID, WavCache, 4);
                        ChunkIdx = WAV_CHUNK_DATASIZE;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get 'fact' error\r\n");
                        return (WAV_DECODE_ERROR_FACTID);
                    }
                }
                break;
            case WAV_CHUNK_FACTSIZE:
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* get fact content size */
                    header->FactSize =  (uint32_t)WavCache[3]<<24;
                    header->FactSize |= (uint32_t)WavCache[2]<<16;
                    header->FactSize |= (uint32_t)WavCache[1]<<8;
                    header->FactSize |= (uint32_t)WavCache[0];
                    ChunkIdx++;
                    if(DataOfs+header->FactSize > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                }
                break;
            case WAV_CHUNK_FACT:
                {
                    for(cidx = 0; cidx < header->FactSize; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* get fact content */
                    ret = wav_GetFactContent(&header->Fact, WavCache, header->FactSize);
                    if(ret == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get fact content error, ret = %d\r\n", ret);
                        ret = WAV_DECODE_ERROR_FACT;
                    }
                }
                break;
            case WAV_CHUNK_DATAID://"data"
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* check 'data' */
                    //printf("header->DataID : %s\r\n", header->DataID);
                    if(myMemcmp(WavCache, "data", 4) == 0)
                    {
                        ChunkIdx++;
                        if(DataOfs+4 > pSize) {return (WAV_DECODE_ERROR_BUFFER);}
                    }
                    else
                    {
                        printf("wav get 'data' error\r\n");
                        return (WAV_DECODE_ERROR_DATAID);
                    }
                }
                break;
            case WAV_CHUNK_DATASIZE:
                {
                    for(cidx = 0; cidx < 4; cidx++)
                    {
                        WavCache[cidx] = pData[DataOfs++];
                    }
                    /* get audio data size */
                    header->AudioDataSize =  (uint32_t)WavCache[3]<<24;
                    header->AudioDataSize |= (uint32_t)WavCache[2]<<16;
                    header->AudioDataSize |= (uint32_t)WavCache[1]<<8;
                    header->AudioDataSize |= (uint32_t)WavCache[0];
                    /* audio data offset in file */
                    *bDecode = DataOfs;
                    /* get header info done */
                    return (WAV_DECODE_ERROR_NONE);
                }
                //break;
            default:
                printf("invalid chunk index, idx = %d\r\n", WavChunkIdx);
                ret = WAV_DECODE_ERROR_FRAME;
        }//switch
    }//while(WAV_DECODE_ERROR_NONE)

    return ret;
}

/*  @param bFill : number of bytes be filled into pBuff
*/
uint8_t Wav_AudioDataProc(WavHeader_t *header, uint8_t *pBuff, uint32_t buffSize, const uint8_t *pData, uint32_t dataSize, uint32_t *bFill)
{
	uint32_t DataOfs = 0, BuffOfs = 0;

    /* check param */
    if((pData == NULL) || (pBuff == NULL) || (dataSize > buffSize)) 
    {
        return (WAV_DECODE_ERROR_PARAM);
    }

   	//printf("buffSize = %d, dataSize = %d\r\n", buffSize, dataSize);
    /* decode */
    switch(header->Fmt.BitsPerSample)
    {
        case 32://4 bytes
        case 16://2 bytes
        case 12://2 bytes
        case 8: //1 byte
        case 4: //1 byte
            {
                myMemcpy(pBuff, pData, dataSize);
                *bFill += dataSize;
            }
            break;
        case 20:
        case 24://3 bytes, 4th byte is filled with '0'
            {	   
                /* size : 3byte audData -> 4byte pBuff */                  
                while((DataOfs < dataSize) && (BuffOfs < buffSize))
                {
                    pBuff[BuffOfs++] = pData[DataOfs++]; //frame byte[0]
                    pBuff[BuffOfs++] = pData[DataOfs++]; //frame byte[1]
                    pBuff[BuffOfs++] = pData[DataOfs++]; //frame byte[2]
                    pBuff[BuffOfs++] = 0;  				//frame byte[3] : 0
                }

                *bFill += BuffOfs;
            }
            break;
        /* unsupport data size */    
        default: return (WAV_DECODE_ERROR_FRAME);
    }

    return (WAV_DECODE_ERROR_NONE);
}

/***************** PRIVATE FUNCTION ******************/

static uint8_t wav_GetFmtContent(WavFmt_t *fmt, uint8_t *fmtBuff, uint32_t size)
{
    uint8_t ret = 0;
    uint8_t tmp[8], i = 0;
    uint32_t totalSize = size;

    if((fmtBuff == NULL) || (size < 2) || (fmt == NULL))
    {
        //error
        return (1);
    }

    /* get encoder format */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if(size < 2)    return (2);//NumChannels byte  
    fmt->EncodeFmt =  (uint16_t)tmp[1] << 16;
    fmt->EncodeFmt |= (uint16_t)tmp[0];
	//printf("EncodeFmt = %d\r\n", fmt->EncodeFmt);
	
    /* get channel numbers */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if(size < 4)    return (3);//SampleRate byte
    fmt->NumChannels =  (uint16_t)tmp[1] << 8;
    fmt->NumChannels |= (uint16_t)tmp[0];
	//printf("NumChannels = %d\r\n", fmt->NumChannels);
	
	/* get sample rate */
	for(i = 0; (i < 4) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if(size < 4)    return (4);//byte per sample rate
    fmt->SampleRate =  (uint16_t)tmp[3] << 24;
    fmt->SampleRate |= (uint16_t)tmp[2] << 16;
    fmt->SampleRate |= (uint16_t)tmp[1] << 8;
    fmt->SampleRate |= (uint16_t)tmp[0];
	//printf("SampleRate = %d\r\n", fmt->SampleRate);
	
    /* get byte rate */
    for(i = 0; (i < 4) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if(size < 2)    return (5);//BlockAlign byte
    fmt->ByteRate =  (uint16_t)tmp[3] << 24;
    fmt->ByteRate |= (uint16_t)tmp[2] << 16;
    fmt->ByteRate |= (uint16_t)tmp[1] << 8;
    fmt->ByteRate |= (uint16_t)tmp[0];
	//printf("ByteRate = %d\r\n", fmt->ByteRate);
	
    /* get block align */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if(size < 2)    return (6);//BitsPerSample byte
    fmt->BlockAlign =  (uint16_t)tmp[1] << 8;
    fmt->BlockAlign |= (uint16_t)tmp[0];
	//printf("BlockAlign = %d\r\n", fmt->BlockAlign);
	
    /* get BitsPerSample */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    if((size < 2) && (totalSize > 16))  return (7);//ExpandSize byte
    fmt->BitsPerSample =  (uint16_t)tmp[1] << 8;
    fmt->BitsPerSample |= (uint16_t)tmp[0];
	//printf("BitsPerSample = %d\r\n", fmt->BitsPerSample);

    /* get expand contend size */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *fmtBuff++;
        size--;
    }
    fmt->ExpandSize =  (uint16_t)tmp[1] << 8;
    fmt->ExpandSize |= (uint16_t)tmp[0];
	//printf("ExpandSize = %d\r\n", fmt->ExpandSize);
	
    /* get expand content */
    if(fmt->ExpandSize)
    {
        for(i = 0; (i < fmt->ExpandSize) && (size); i++)
        {
            tmp[i] = *fmtBuff++;
            size--;
        }
        ret = wav_GetExpandContent(&fmt->Expand, fmtBuff, fmt->ExpandSize);
        if(ret == 0)
        {
            fmt->EncodeFmt = fmt->Expand.EncodeFmt;
            if(fmt->BitsPerSample == 0)
            {
                fmt->BitsPerSample = fmt->Expand.IBitsPerSample;
            }
        }
        else
        {
            printf("get expand context error, ret = %d\r\n", ret);
            return (8);
        }
    }

    return (0);
}

static uint8_t wav_GetExpandContent(WavExpand_t *expand, uint8_t *expBuff, uint32_t size)
{
    uint8_t tmp[8], i = 0;

    if((expBuff == NULL) || (size < 2) || (expand == NULL))
    {
        //error
        return (1);
    }
    /* get valid BitsPerSample */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *expBuff++;
        size--;
    }
    if(size < 4)    return (2);//ChannelMask byte
    expand->IBitsPerSample =  (uint16_t)tmp[1] << 8;
    expand->IBitsPerSample |= (uint16_t)tmp[0];
    
    /* get ChannelMask */
    for(i = 0; (i < 4) && (size); i++)
    {
        tmp[i] = *expBuff++;
        size--;
    }
    if(size < 2)    return (3);//EncodeFmt byte
    expand->ChannelMask =  (uint16_t)tmp[3] << 24;
    expand->ChannelMask |= (uint16_t)tmp[2] << 16;
    expand->ChannelMask |= (uint16_t)tmp[1] << 8;
    expand->ChannelMask |= (uint16_t)tmp[0];
    
    /* get encoder format */
    for(i = 0; (i < 2) && (size); i++)
    {
        tmp[i] = *expBuff++;
        size--;
    }
    //if(size < 14) return (4);//reserve byte
    expand->EncodeFmt |= (uint16_t)tmp[1] << 8;
    expand->EncodeFmt |= (uint16_t)tmp[0];

    return (0);
}

static uint8_t wav_GetFactContent(WavFact_t *fact, uint8_t *factBuff, uint32_t size)
{
    uint8_t tmp[8], i = 0;

    if((factBuff == NULL) || (size < 4) || (fact == NULL))
    {
        //error
        return (1);
    }

    for(i = 0; i < 4; i++)
    {
        tmp[i] = *factBuff++;
        size--;
    }

    fact->BytesPerSample =  (uint16_t)tmp[3] << 24;
    fact->BytesPerSample |= (uint16_t)tmp[2] << 16;
    fact->BytesPerSample |= (uint16_t)tmp[1] << 8;
    fact->BytesPerSample |= (uint16_t)tmp[0];

    return (0);
}


