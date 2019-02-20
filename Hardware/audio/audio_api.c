
#include "stm32h7xx.h"
#include "audio_api.h"
#include "audio_sai.h"
#include "WM8978G.h"

int16_t audioErrorCode = AUDIO_ERROR_NONE;
int16_t audioStateCode = AUDIO_STATE_NOINIT;

volatile bool CodecForceStop = FALSE;
volatile uint32_t CodecTotalBytes = 0;
volatile uint32_t CodecXsferBytes = 0;
volatile uint32_t CodecByteRate = 0;

/* @return : 0-OK, 1-Unsupport
*/
uint8_t Audio_Init(void)
{    
    uint8_t ret = 0;
    ret = Audio_SAILowLevelInit();
    if(ret != 0) 
    {  
        audioErrorCode = AUDIO_ERROR_ITFACE;
        return (AUDIO_RES_ERROR);
    }
    
    ret = Audio_AmpInit();
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_INIT;
    return (AUDIO_RES_OK);
}

uint8_t Audio_DeInit(void)
{
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_NOINIT;
    return (AUDIO_RES_OK);
}

uint8_t Audio_SetMute(bool mute)
{
    uint8_t ret = 0;
    
    ret = Audio_AmpSetMute(mute);
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    return (AUDIO_RES_OK);
}

uint8_t Audio_SetVolume(uint8_t vol)
{
    uint8_t ret = 0;

    ret = Audio_AmpSetVolume(vol);
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    return (AUDIO_RES_OK);
}

uint8_t Audio_SetEQ(uint8_t eq)
{
    uint8_t ret = 0;

    ret = Audio_AmpSetEQ(eq);
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    return (AUDIO_RES_OK);
}

uint8_t Audio_SetChannel(uint8_t ch)
{
    uint8_t ret = 0;

    ret = Audio_AmpSetChannel(ch);
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    return (AUDIO_RES_OK);
}

uint8_t Audio_DecodeInit(AudioDecInit_t *Init)
{
    uint8_t ret = 0;
    uint32_t SAI_DataWidth = 0, SAI_SampleRate = 0, SAI_Csb = 0;
    uint32_t Codec_DataWidth = 0, Codec_SampleRate = 0;
    uint8_t Codec_Protocal = 0;
    SAITxInit_t SAITx_Init;

	//printf("audioStateCode = %d\r\n", audioStateCode);
    if((audioStateCode == AUDIO_STATE_NOINIT)
        || (audioStateCode == AUDIO_STATE_ERROR))
    {
        return (AUDIO_RES_ERROR);
    }

    if(!Init->FillTxBuffer && !Init->SwitchTxBuffer) 
    {
        audioErrorCode = AUDIO_ERROR_STREAM;
        return (AUDIO_RES_ERROR);
    }
    
    SAI_Csb = SAI_CLOCKSTROBING_FALLINGEDGE;
    SAI_SampleRate = Init->sampleRate;
    Codec_SampleRate = Init->sampleRate;
    Codec_Protocal = CODEC_PROTOCAL_I2S_STANDARD;

    switch (Init->dataSize)
    {
        case 24:
            //SAI_DataWidth = SAI_DATASIZE_24;
            //Codec_DataWidth = CODEC_DATASIZE_24B;
            //break;
        case 32:
            SAI_DataWidth = SAI_DATASIZE_32;
            Codec_DataWidth = CODEC_DATASIZE_32B;
            //return (AUDIO_ERROR_UNSUPPORT_DATASIZE);
            break;
        //case 8:
        case 16:
            SAI_DataWidth = SAI_DATASIZE_16;
            Codec_DataWidth = CODEC_DATASIZE_16B;        
            break;
        default:
            audioErrorCode = AUDIO_ERROR_UNSUPPORT_DATASIZE;
            return (AUDIO_RES_ERROR);
    }
    
    ret = Audio_AmpI2SConfig(Codec_Protocal, Codec_SampleRate, Codec_DataWidth);
    if(ret != 0) {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }

    SAITx_Init.buffer0 = Init->buffer0;
    SAITx_Init.buffer1 = Init->buffer1;
    SAITx_Init.buffSize = Init->buffSize;
    SAITx_Init.rdataSize = Init->dataSize;
    SAITx_Init.tdataSize = SAI_DataWidth;
    SAITx_Init.frequency = SAI_SampleRate;
    SAITx_Init.clkStrobing = SAI_Csb;
    SAITx_Init.FillTxBuffer = Init->FillTxBuffer;
    SAITx_Init.SwitchTxBuffer = Init->SwitchTxBuffer;
    ret = Audio_SAITxConfig(&SAITx_Init);
    if(ret != 0) {
        audioErrorCode = AUDIO_ERROR_ITFACE;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    CodecTotalBytes = Init->totalBytes;//if set to 0, SAI will never end
    CodecXsferBytes = 0;
    CodecByteRate = Init->byteRate;
    audioStateCode = AUDIO_STATE_READY;
    audioErrorCode = AUDIO_ERROR_NONE;
    
    return (AUDIO_RES_OK);
}

uint8_t Audio_PlayInit(void)
{
    uint8_t ret = 0;

    ret = Audio_AmpPlayStart();
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }
		
    return (AUDIO_RES_OK);
}

uint8_t Audio_SingalBufferPlayStart(uint8_t *pBuffer, uint32_t pSize)
{
    uint8_t ret = 0;

    ret = Audio_SAIDataXsfer(pBuffer, pSize);
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_ITFACE;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    CodecForceStop = FALSE;
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_PLAY;
    return (AUDIO_RES_OK);    
}

uint8_t Audio_DoubleBufferPlayStart(void)
{
    uint8_t ret = 0;

    ret = Audio_SAIMultiBufferTxStart();
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_ITFACE;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    CodecForceStop = FALSE;
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_PLAY;
    return (AUDIO_RES_OK);
}

uint8_t Audio_PlayStop(void)
{
    uint8_t ret = 0;

    ret = Audio_AmpPlayStop();
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_AMP;
        return (AUDIO_RES_ERROR);
    }
    
    ret = Audio_SAITxStop();
    if(ret != 0) 
    {
        audioErrorCode = AUDIO_ERROR_ITFACE;
        return (AUDIO_RES_ERROR);
    }

    /* error none */
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_READY;
    CodecForceStop = TRUE;
    return (AUDIO_RES_OK);    
}

uint32_t Audio_GetXsferBytes(void)
{
    return CodecXsferBytes;
}

/* in second */
uint32_t Audio_GetCodecTime(void)
{
    return CodecXsferBytes / CodecByteRate;
}

uint8_t Audio_GetError(void)
{
    return audioErrorCode;
}

uint8_t Audio_GetState(void)
{
    return audioStateCode;
}

#if 0
/* audio driver register */
uint8_t AudioDrv_Register(AudioDecDrv_t *drv)
{
    if(!drv) {return (1);}

    drv->Init = Audio_Init;
    drv->DeInit = Audio_DeInit;
    drv->SetMute = Audio_SetMute;
    drv->SetVolume = Audio_SetVolume;
    drv->SetEQ = Audio_SetEQ;
    drv->SetChannel = Audio_SetChannel;
    drv->DecodeInit = Audio_DecodeInit;
    drv->PlayStart = Audio_PlayStart;
    drv->PlayStop = Audio_PlayStop;
    drv->GetXsferBytes = Audio_GetXsferBytes;
    drv->GetCodecTime = Audio_GetCodecTime;
    drv->GetState = Audio_GetState;
    drv->GetError = Audio_GetError;
    drv->state = AUDIODRV_REGISTED;
    
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_NOINIT;
    
    return (AUDIO_RES_ERROR);
}

void AudioDrv_DeRegister(AudioDecDrv_t *drv)
{
    drv->Init = NULL;
    drv->DeInit = NULL;
    drv->SetMute = NULL;
    drv->SetVolume = NULL;
    drv->SetEQ = NULL;
    drv->SetChannel = NULL;
    drv->DecodeInit = NULL;
    drv->PlayStart = NULL;
    drv->PlayStop = NULL;
    drv->GetXsferBytes = NULL;
    drv->GetState = NULL;
    drv->GetError = NULL;
    drv->state = AUDIODRV_NOTREG;
    
    audioErrorCode = AUDIO_ERROR_NONE;
    audioStateCode = AUDIO_STATE_NOINIT;
}
#endif


