
#ifndef _AUDIO_API_H
#define _AUDIO_API_H

#include "audio_sai.h"
#include "public.h"


/* Audio state code
*/
typedef enum
{
    AUDIO_STATE_NOINIT = 0,
    AUDIO_STATE_INIT,
    AUDIO_STATE_READY,
    AUDIO_STATE_PLAY,
    AUDIO_STATE_RECORD,
    AUDIO_STATE_STREAM_END,

    AUDIO_STATE_ERROR = 255
}AudioState;

/*  Audio error code
*/
typedef enum
{
    AUDIO_ERROR_NONE = 0,
    AUDIO_ERROR_AMP,
    AUDIO_ERROR_DRVIC,//CODEC IC Error
    AUDIO_ERROR_ITFACE,//SAI Error
    AUDIO_ERROR_TIMEOUT,
    AUDIO_ERROR_UNSUPPORT_DATASIZE,
    AUDIO_ERROR_UNSUPPORT_FREQ,
    AUDIO_ERROR_UNSUPPORT_PROTOCAL,
    AUDIO_ERROR_STREAM,
    AUDIO_ERROR_TX,
    AUDIO_ERROR_RX,

    AUDIO_ERROR_NOINIT = 255
}AudioError;

typedef enum
{
    AUDIO_RES_OK = 0,
    AUDIO_RES_ERROR,
}AudioResult;

/* codec input channel
*/
#define CODEC_INPUT_NONE            (0x00)
#define CODEC_INPUT_MIC             (0x01)
#define CODEC_INPUT_LINEIN	        (0x02)
#define CODEC_INPUT_AUX		        (0x04)
/* codec output channel
*/
#define CODEC_OUTPUT_NONE	        (0x00)
#define CODEC_OUTPUT_HP		        (0x01)
#define CODEC_OUTPUT_SPK	        (0x02)
#define CODEC_OUTPUT_ALL            (CODEC_OUTPUT_HP | CODEC_OUTPUT_SPK)

/* Audio IIS Protocal */
#define CODEC_PROTOCAL_I2S_LSB	    (0)
#define CODEC_PROTOCAL_I2S_MSB	    (1)
#define CODEC_PROTOCAL_I2S_STANDARD	(2)
#define CODEC_PROTOCAL_PCM_DSP	    (3)

/* Audio Data size
*/
#define CODEC_DATASIZE_16B          (0)
#define CODEC_DATASIZE_20B	        (1)
#define CODEC_DATASIZE_24B	        (2)
#define CODEC_DATASIZE_32B	        (3)

/* Audio Sample Rate
*/
#define CODEC_FREQUENCY_192K        ((uint32_t)192000u)    
#define CODEC_FREQUENCY_96K         ((uint32_t)96000u)     
#define CODEC_FREQUENCY_48K         ((uint32_t)48000u)
#define CODEC_FREQUENCY_44K         ((uint32_t)44100u)   
#define CODEC_FREQUENCY_32K         ((uint32_t)32000u)
#define CODEC_FREQUENCY_22K         ((uint32_t)22050u)        
#define CODEC_FREQUENCY_16K         ((uint32_t)16000u)
#define CODEC_FREQUENCY_11K         ((uint32_t)11025u)       
#define CODEC_FREQUENCY_8K          ((uint32_t)8000u)     

typedef struct
{
    uint32_t totalBytes;//if set to 0, SAI will never auto stop.
    uint32_t sampleRate;
    uint32_t byteRate;
    uint8_t protocal;
    uint8_t  dataSize;

    uint8_t *buffer0;
    uint8_t *buffer1;
    uint32_t buffSize;
    SAIFillTxBufferFunc FillTxBuffer;//called when use double buffer
    SAISwitchTxBufferFunc SwitchTxBuffer;//called when use singal buffer
}AudioDecInit_t;

#define BUFFER_FULL0     (0x01)
#define BUFFER_FULL1     (0x02)

#if 0
/* audio driver */
typedef struct
{
    uint8_t (*Init)(void);
    uint8_t (*DeInit)(void);
    uint8_t (*SetMute)(bool mute);
    uint8_t (*SetVolume)(uint8_t vol);
    uint8_t (*SetEQ)(uint8_t eq);
    uint8_t (*SetChannel)(uint8_t ch);
    uint8_t (*DecodeInit)(AudioDecInit_t *DecInfo);
    uint8_t (*PlayStart)(void);
    uint8_t (*PlayStop)(void);
    uint32_t(*GetXsferBytes)(void);
    uint32_t(*GetCodecTime)(void);
    int16_t (*GetState)(void);
    int16_t (*GetError)(void);

    uint8_t state;
}AudioDecDrv_t;

#define AUDIODRV_NOTREG     (0)
#define AUDIODRV_REGISTED   (1)

#endif

extern int16_t audioErrorCode;
extern int16_t audioStateCode;
extern volatile bool CodecForceStop;
extern volatile uint32_t CodecTotalBytes;
extern volatile uint32_t CodecXsferBytes;
extern volatile uint32_t CodecByteRate;

extern uint8_t Audio_Init(void);
extern uint8_t Audio_DeInit(void);
extern uint8_t Audio_SetMute(bool mute);
extern uint8_t Audio_SetVolume(uint8_t vol);
extern uint8_t Audio_SetEQ(uint8_t eq);
extern uint8_t Audio_SetChannel(uint8_t ch);
extern uint8_t Audio_DecodeInit(AudioDecInit_t *Init);
extern uint8_t Audio_PlayInit(void);
extern uint8_t Audio_SingalBufferPlayStart(uint8_t *pBuffer, uint32_t pSize);
extern uint8_t Audio_DoubleBufferPlayStart(void);
extern uint8_t Audio_PlayStop(void);
extern uint32_t Audio_GetXsferBytes(void);
extern uint32_t Audio_GetCodecTime(void);
extern uint8_t Audio_GetError(void);
extern uint8_t Audio_GetState(void);

/* Decoder IC */
extern uint8_t Audio_DecoderStart(void);
extern uint8_t Audio_DecoderStop(void);
#if 0
extern uint8_t AudioDrv_Register(AudioDecDrv_t *drv);
extern void AudioDrv_DeRegister(AudioDecDrv_t *drv);
#endif

#endif

