
#include "stm32h7xx.h"
#include "audio_sai.h"
#include "audio_api.h"
#include "led.h"

#define AudioSAI_EnableDMA()	{AUDIO_SAI->CR1 |= SAI_xCR1_DMAEN;}
#define AudioDMA_BufferIdx()    ((AUDIO_DMA->CR & (1<<19)) ? 1 : 0)

/* SAI clock config */
typedef struct
{
    //uint8_t  Channel;
    //uint8_t  Bits;
    uint16_t SampleRate;//samplerate / 10
    uint16_t PLLN;
    uint16_t PLLP;
    uint16_t MCKDIV;
    uint16_t OSR;//over sample
}SampleRateConf_t;

/* SAI sample rate setting table
*   MCKDIVEN!=0: Fs=SAI_CK_x/[512*MCKDIV]
*   MCKDIVEN==0: Fs=SAI_CK_x/256
*   SAI_CK_x = (HSE/pllm)*PLLSAIN/(PLLSAIP+1)
*  SAI Core Clock : 1MHz [pllm = 25MHz, HSE = 25MHz]
*/
const SampleRateConf_t SAI_SRConf[]=
{
//freq/10,PLLN,PLLP,MCKDIV,OSR
	{800 ,  215, 105, 5, 1},	//8Khz      sample rate
	{1102,  333,  59, 1, 1},	//11.025Khz sample rate 
	{1600,  467,  57, 1, 1},	//16Khz     sample rate
	{2205,  429,  38, 1, 1},	//22.05Khz  sample rate
	{3200,  426,  26, 1, 1},	//32Khz     sample rate
	{4410,  429,  38, 1, 0},	//44.1Khz   sample rate
	{4800,  467,  32, 1, 0},	//48Khz     sample rate
	{8820,  429,  19, 1, 0},	//88.2Khz   sample rate
	{9600,  467,  19, 1, 0},	//96Khz     sample rate
	{17640, 497,  11, 1, 0},	//176.4Khz  sample rate 
	{19200, 442,   9, 1, 0},	//192Khz    sample rate
};

#define SAI_SRCFG_TALSIZE   (sizeof(SAI_SRConf)/sizeof(SampleRateConf_t))

/* HAL Handler */
static SAI_HandleTypeDef AudioSAI_Handle;
static DMA_HandleTypeDef AudioDMA_Handle;
/* DMA State
*/
//static bool IsMultiBuffer = FALSE;
static SAIFillTxBufferFunc AudioFillTxBuffer;//called when use double buffer
static SAISwitchTxBufferFunc AudioSwitchTxBuffer;//called when use singal buffer
    
static uint8_t *AudioBuffer0;
static uint8_t *AudioBuffer1;
static uint32_t AudioBufferSize;
static uint8_t AudioDataSize;
//static uint32_t AudioFrequency;
static uint32_t SAITargBufferSize;

/*** SAI Functions *****/

uint8_t Audio_SAILowLevelInit(void)
{
	GPIO_InitTypeDef GPIO_Init;

	/* GPIO Ports Clock Enable */
  	__HAL_RCC_GPIOE_CLK_ENABLE();
 	/**SAI1_A_Block_A GPIO Configuration    
    PE2     ------> SAI1_MCLK_A
    PE4     ------> SAI1_FS_A
    PE5     ------> SAI1_SCK_A
    PE6     ------> SAI1_SD_A 
    */
    GPIO_Init.Pin = GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOE, &GPIO_Init);

    /* SAI DMA IRQ Enable */
    HAL_NVIC_SetPriority(AUDIO_DMAIRQn, 2, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DMAIRQn);
    __HAL_RCC_DMA1_CLK_ENABLE();
	
    return (0);
}

uint8_t Audio_SAILowLevelDeInit(void)
{
    __HAL_RCC_GPIOE_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(AUDIO_DMAIRQn);
    __HAL_RCC_DMA1_CLK_DISABLE();

    return (0);
}

uint8_t Audio_SAITxConfig(SAITxInit_t *Init)
{	
    uint8_t idx = 0, ret = 0;
    uint32_t MemDataAlign, PerDataAlign;
    RCC_PeriphCLKInitTypeDef SAICLK_Init;
    
    /* HAL SAI Core clock */
    assert_param(AUDIO_SAI_CORE_CLOCK == RCC_SAI1CLKSOURCE_PLL2);
    for(idx = 0; idx < SAI_SRCFG_TALSIZE; idx++) {
        if((Init->frequency / 10) == SAI_SRConf[idx].SampleRate) {
            break;
        }
    }
    /* not find */
    if(idx == SAI_SRCFG_TALSIZE) {
        return (1);
    }
    
    SAICLK_Init.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
	SAICLK_Init.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
	SAICLK_Init.PLL2.PLL2M = 25;
	SAICLK_Init.PLL2.PLL2N = (uint32_t)SAI_SRConf[idx].PLLN;
	SAICLK_Init.PLL2.PLL2P = (uint32_t)SAI_SRConf[idx].PLLP;
	ret = HAL_RCCEx_PeriphCLKConfig(&SAICLK_Init);
	if(ret != HAL_OK) {
	    return (2);
    }

    /************ SAI ****************************/
    __HAL_RCC_SAI1_CLK_ENABLE();
    /* Instance */
    AudioSAI_Handle.Instance = AUDIO_SAI;
    HAL_SAI_DeInit(&AudioSAI_Handle);
	/* Init */
	AudioSAI_Handle.Init.AudioMode = SAI_MODEMASTER_TX;
	AudioSAI_Handle.Init.Synchro = SAI_ASYNCHRONOUS;
	AudioSAI_Handle.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
	AudioSAI_Handle.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	AudioSAI_Handle.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_HF;
	AudioSAI_Handle.Init.AudioFrequency = Init->frequency;
	AudioSAI_Handle.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
	AudioSAI_Handle.Init.MonoStereoMode = SAI_STEREOMODE;
	//AudioSAI_Handle.Init.CompandingMode = SAI_NOCOMPANDING;
	//AudioSAI_Handle.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	AudioSAI_Handle.Init.ClockStrobing = Init->clkStrobing;
	/* solt, SAI transmit one channel data in one slot */
	AudioSAI_Handle.SlotInit.FirstBitOffset = 0;
	AudioSAI_Handle.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	AudioSAI_Handle.SlotInit.SlotNumber = 2;
	AudioSAI_Handle.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;
	/* format */
	AudioSAI_Handle.Init.Protocol = SAI_FREE_PROTOCOL;
	AudioSAI_Handle.Init.FirstBit = SAI_FIRSTBIT_MSB;
	AudioSAI_Handle.Init.DataSize = Init->tdataSize;
	/* over sample */
	if(SAI_SRConf[idx].OSR) {
		AudioSAI_Handle.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_ENABLE;
	} else {
		AudioSAI_Handle.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
    }
	/* frame */
    switch(Init->tdataSize)
	{
	    case SAI_DATASIZE_8:
	    case SAI_DATASIZE_10:
	    case SAI_DATASIZE_16:
	        MemDataAlign = DMA_MDATAALIGN_HALFWORD;
			PerDataAlign = DMA_PDATAALIGN_HALFWORD;
			SAITargBufferSize = Init->buffSize / 2;
            AudioSAI_Handle.FrameInit.FrameLength = 32;
    	    AudioSAI_Handle.FrameInit.ActiveFrameLength = 16;//FrameLength / 2
	        break;
	    case SAI_DATASIZE_20:
	    case SAI_DATASIZE_24:
	    case SAI_DATASIZE_32:
	        MemDataAlign = DMA_MDATAALIGN_WORD;
			PerDataAlign = DMA_PDATAALIGN_WORD;
			SAITargBufferSize = Init->buffSize / 4;
            AudioSAI_Handle.FrameInit.FrameLength = 64;
    	    AudioSAI_Handle.FrameInit.ActiveFrameLength = 32;//FrameLength / 2
	        break;
	}
	AudioSAI_Handle.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	AudioSAI_Handle.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	AudioSAI_Handle.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
	ret = HAL_SAI_Init(&AudioSAI_Handle);
	if(ret != HAL_OK) {
	    return (3);
	}

    /********** DMA Config ************************/
    /* Instance */
	AudioDMA_Handle.Instance = AUDIO_DMA;
	HAL_DMA_DeInit(&AudioDMA_Handle);
	//__HAL_LINKDMA(&AudioSAI_Handle, hdmatx, AudioDMA_Handle);
	AudioDMA_Handle.Init.Request = DMA_REQUEST_SAI1_A;
	AudioDMA_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	AudioDMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	AudioDMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
	AudioDMA_Handle.Init.PeriphDataAlignment = PerDataAlign;
	AudioDMA_Handle.Init.MemDataAlignment = MemDataAlign;
	AudioDMA_Handle.Init.Mode = DMA_CIRCULAR;
	AudioDMA_Handle.Init.Priority = DMA_PRIORITY_MEDIUM;
	AudioDMA_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	AudioDMA_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
	AudioDMA_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
	AudioDMA_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
	ret = HAL_DMA_Init(&AudioDMA_Handle);
	if(ret != HAL_OK) {
	    return (4);
	}
    /*  */
	AudioSAI_EnableDMA();
	/* DMA buffer */

    AudioBuffer0 = Init->buffer0;
    AudioBuffer1 = Init->buffer1;
    AudioBufferSize = Init->buffSize;
    AudioFillTxBuffer = Init->FillTxBuffer;
    AudioSwitchTxBuffer = Init->SwitchTxBuffer;
    AudioDataSize = Init->rdataSize;
    //AudioFrequency = Init->frequency;  
    
    if(AudioBuffer0 && AudioBuffer1)
    {
    	ret = HAL_DMAEx_MultiBufferStart(&AudioDMA_Handle, (uint32_t)AudioBuffer0, (uint32_t)&(AUDIO_SAI->DR), 
    	                                    (uint32_t)AudioBuffer1, SAITargBufferSize);
   		if(ret != HAL_OK) {
   		    return (5);
   		}
        //IsMultiBuffer = TRUE;
    }
    else
    {
        //IsMultiBuffer = FALSE;
    }

    if(!AudioFillTxBuffer && !AudioSwitchTxBuffer){
        return (6);
    }

	return (0);
}

uint8_t Audio_SAIDataXsfer(uint8_t *pData, uint32_t pSize)
{
    /* SAI Start */	
	__HAL_SAI_ENABLE(&AudioSAI_Handle);
	/* DMA IT Init */
    __HAL_DMA_ENABLE_IT(&AudioDMA_Handle, DMA_IT_TC|DMA_IT_TE|DMA_IT_FE);
	__HAL_DMA_CLEAR_FLAG(&AudioDMA_Handle, DMA_FLAG_TCIF1_5|DMA_FLAG_TEIF1_5|DMA_FLAG_FEIF1_5);

    HAL_DMA_Start(&AudioDMA_Handle, (uint32_t)pData, (uint32_t)&(AUDIO_SAI->DR), pSize);    
	__HAL_DMA_ENABLE(&AudioDMA_Handle);
	
    return (0);
}

uint8_t Audio_SAIMultiBufferTxStart(void)
{
    /* SAI Start */	
	__HAL_SAI_ENABLE(&AudioSAI_Handle);
	/* DMA IT Init */
    __HAL_DMA_ENABLE_IT(&AudioDMA_Handle, DMA_IT_TC|DMA_IT_TE|DMA_IT_FE);
	__HAL_DMA_CLEAR_FLAG(&AudioDMA_Handle, DMA_FLAG_TCIF1_5|DMA_FLAG_TEIF1_5|DMA_FLAG_FEIF1_5);
	__HAL_DMA_ENABLE(&AudioDMA_Handle);

	return (0);
}

uint8_t Audio_SAITxStop(void)
{
	__HAL_DMA_CLEAR_FLAG(&AudioDMA_Handle, DMA_FLAG_TCIF1_5|DMA_FLAG_TEIF1_5|DMA_FLAG_FEIF1_5);
	__HAL_DMA_DISABLE_IT(&AudioDMA_Handle, DMA_IT_TC|DMA_IT_TE|DMA_IT_FE);
	__HAL_DMA_DISABLE(&AudioDMA_Handle);
	__HAL_SAI_DISABLE(&AudioSAI_Handle);
	
	return (0);
}

/* Audio DMA IRQ Handler */
void DMA1_Stream5_IRQHandler(void)
{
    uint8_t cbret = 0, idx = 0;
	//LED_Toggle(LED_GREEN);
	/* tx completed */
	if(__HAL_DMA_GET_FLAG(&AudioDMA_Handle, DMA_FLAG_TCIF1_5) != RESET)
	{
		__HAL_DMA_CLEAR_FLAG(&AudioDMA_Handle, DMA_FLAG_TCIF1_5);

		if(audioStateCode == AUDIO_STATE_STREAM_END) {
		    return ;
		}
		if(CodecForceStop == TRUE) {
		    return ;
		}
		
        CodecXsferBytes += AudioBufferSize;
        
        if((CodecTotalBytes) && (CodecXsferBytes >= CodecTotalBytes))
        {
          	audioErrorCode = AUDIO_ERROR_NONE;
          	audioStateCode = AUDIO_STATE_STREAM_END;
          	__HAL_DMA_DISABLE(&AudioDMA_Handle);
        }

        //if(IsMultiBuffer) //Multiple buffer tx
        {
            idx = AudioDMA_BufferIdx();//buffer in transfering
    		if(idx == 0) 
    		{		    
    		    cbret = AudioFillTxBuffer(AudioBuffer1, AudioBufferSize, AudioDataSize);
                if(cbret == FILL_BUFF_ERR)
                {
                	printf("%s", "SAI DMA Tx buffer1 error!\r\n");
                    audioErrorCode = AUDIO_ERROR_STREAM;
                    audioStateCode = AUDIO_STATE_ERROR;
                    __HAL_DMA_DISABLE(&AudioDMA_Handle);
                }
    		}
    		else if(idx == 1) 
    		{   		    
    		    cbret = AudioFillTxBuffer(AudioBuffer0, AudioBufferSize, AudioDataSize);
                if(cbret == FILL_BUFF_ERR)
                {
                	printf("%s", "SAI DMA Tx buffer0 error!\r\n");
                    audioErrorCode = AUDIO_ERROR_STREAM;
                    audioStateCode = AUDIO_STATE_ERROR;
                    __HAL_DMA_DISABLE(&AudioDMA_Handle);
                }
    		}
            else 
            {
            	printf("%s", "SAI DMA Tx buffer idx error!\r\n");
                audioErrorCode = AUDIO_ERROR_TX;
                audioStateCode = AUDIO_STATE_ERROR;
                __HAL_DMA_DISABLE(&AudioDMA_Handle);
            }
        }
        //else //singal buffer tx
        //{
        //    if(AudioSwitchTxBuffer)
        //    {
        //        AudioSwitchTxBuffer();
        //    }
        //}
	}
	/* tx error */
	if(__HAL_DMA_GET_FLAG(&AudioDMA_Handle, DMA_FLAG_TEIF1_5) != RESET)
	{
		printf("%s", "SAI DMA Tx error!\r\n");
		audioErrorCode = AUDIO_ERROR_TX;
        audioStateCode = AUDIO_STATE_ERROR;
        __HAL_DMA_CLEAR_FLAG(&AudioDMA_Handle, DMA_FLAG_TEIF1_5);
        __HAL_DMA_DISABLE(&AudioDMA_Handle);
	}
}

