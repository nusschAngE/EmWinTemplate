
#include "stm32h7xx.h"
#include "sdcard.h"
#include "delay.h"

#define SDCardIO                SDMMC1
#define SDCardIRQ               SDMMC1_IRQn

#define SDCardIO_GET_FLAG(__FLAG__)         __SDMMC_GET_FLAG(SDCardIO, __FLAG__)
#define SDCardIO_CLEAR_FLAG(__FLAG__)       __SDMMC_CLEAR_FLAG(SDCardIO, __FLAG__)
#define SDCardIO_ENABLE_IT(__INTERRUPT__)   __SDMMC_ENABLE_IT(SDCardIO, __INTERRUPT__)
#define SDCardIO_DISABLE_IT(__INTERRUPT__)  __SDMMC_DISABLE_IT(SDCardIO, __INTERRUPT__)
#define SDCardIO_TRANSFER_ENABLE()          __SDMMC_CMDTRANS_ENABLE(SDCardIO)
#define SDCardIO_TRANSFER_DISABLE()         __SDMMC_CMDTRANS_DISABLE(SDCardIO)

static HAL_SD_CardInfoTypeDef SDCard;
static uint8_t SDCardInitDone = FALSE;
//static uint32_t SDCID[4];
static uint32_t SDCSD[4];
static __IO uint32_t SDCardContext = 0;
static __IO uint32_t SDCardStatus = HAL_SD_STATE_RESET;
static __IO uint32_t SDCardErrStatus = HAL_SD_ERROR_NONE;
#if (SDCARD_DMA_MODE)
static __IO uint32_t *pTxBuffPtr;
static __IO uint32_t TxBuffSize;
static __IO uint32_t *pRxBuffPtr;
static __IO uint32_t RxBuffSize;
static __IO bool SDReadCompleted = FALSE;
static __IO bool SDWriteCompleted = FALSE;
static __IO bool SDReadWriteError = FALSE;
#endif

/*  STATIC 
*/
static uint32_t SD_InitProcess(void);
static uint32_t SD_PowerON(void);
static uint32_t SD_PowerOFF(void);
static uint32_t SD_Reset(void);
static uint32_t SD_InitCard(void);
static uint32_t SD_EnableWideBus(void);
static uint32_t SD_ConfigClock(uint8_t ClockDiv);
static uint32_t SD_DisableWideBus(void);
static uint32_t SD_GetCardStatus(uint32_t *pCardStatus);
static uint32_t SD_ReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
static uint32_t SD_WriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
#if (SDCARD_DMA_MODE)
static uint32_t SD_ReadBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
static uint32_t SD_WriteBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
#endif
#if 0
static uint32_t SD_Read_IT(void);
static uint32_t SD_Write_IT(void);
#endif
/****************** PUBLIC FUNCTION **********************/
SDBaseInfo_t SDBaseInfo;

/*******************/
/* sd card controller clock & gpio initialize */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{    
    (void)hsd;

	//PLL1_ClocksTypeDef PLL1_Clock;
	GPIO_InitTypeDef GPIO_Init;

    /* Peripheral clock enable */
    __HAL_RCC_SDMMC1_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
  
    /**SDMMC1 GPIO Configuration    
    PC8     ------> SDMMC1_D0
    PC9     ------> SDMMC1_D1
    PC10     ------> SDMMC1_D2
    PC11     ------> SDMMC1_D3
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD 
    */
    GPIO_Init.Pin = GPIO_PIN_8 |GPIO_PIN_9 |GPIO_PIN_10
    				| GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    GPIO_Init.Pin = GPIO_PIN_2;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOD, &GPIO_Init);

#if (SDCARD_DMA_MODE)
    HAL_NVIC_SetPriority(SDCardIRQ, 1, 0);
    HAL_NVIC_EnableIRQ(SDCardIRQ);
#endif
	//HAL_RCCEx_GetPLL1ClockFreq(&PLL1_Clock);
	//printf("SDMMC Core clock = %d - %d - %d\r\n", PLL1_Clock.PLL1_P_Frequency, PLL1_Clock.PLL1_Q_Frequency, PLL1_Clock.PLL1_R_Frequency);
}

#if (SDCARD_DMA_MODE)
void SDMMC1_IRQHandler(void)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    if(SDCardIO_GET_FLAG(SDMMC_FLAG_DATAEND) != RESET)
    {
        /* data transfer end */
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_DATAEND); 
        /* disable interrupt */
        SDCardIO_DISABLE_IT(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR\
                           | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND | SDMMC_FLAG_IDMATE\
                           | SDMMC_FLAG_TXFIFOHE | SDMMC_FLAG_RXFIFOHF);
        /* DMA Transfer */
        if((SDCardContext & SD_CONTEXT_DMA) != RESET)
        {
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardIO_TRANSFER_DISABLE();

            SDCardIO->DLEN = 0;
            SDCardIO->DCTRL = 0;
            SDCardIO->IDMACTRL = SDMMC_DISABLE_IDMA ;
        }
        if(SDCardStatus == HAL_SD_STATE_RECEIVING)
        {
            SDReadCompleted = TRUE;
        }
        if(SDCardStatus == HAL_SD_STATE_TRANSFER)
        {
            SDWriteCompleted = TRUE;
        }

        SDCardStatus = HAL_SD_STATE_READY;
    }
#if 0    
    else if (SDCardIO_GET_FLAG(SDMMC_IT_TXFIFOHE) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_TXFIFOHE);
    
        sd_Write_IT();
    }
    else if (SDCardIO_GET_FLAG(SDMMC_IT_RXFIFOHF) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_RXFIFOHF);
    
        sd_Read_IT();
    }
#endif    
    else if (SDCardIO_GET_FLAG(SDMMC_IT_DCRCFAIL) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_DCRCFAIL);
        SDCardIO_DISABLE_IT(SDMMC_IT_DCRCFAIL);
    	SDCardStatus = HAL_SD_STATE_READY;
        SDReadWriteError = TRUE;
    }
    else if (SDCardIO_GET_FLAG(SDMMC_IT_DTIMEOUT) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_DTIMEOUT);
        SDCardIO_DISABLE_IT(SDMMC_IT_DTIMEOUT);
    	SDCardStatus = HAL_SD_STATE_READY;
        SDReadWriteError = TRUE;
    }
    else if (SDCardIO_GET_FLAG(SDMMC_IT_RXOVERR) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_RXOVERR);
        SDCardIO_DISABLE_IT(SDMMC_IT_RXOVERR);
    	SDCardStatus = HAL_SD_STATE_READY;
        SDReadWriteError = TRUE;
    }
    else if (SDCardIO_GET_FLAG(SDMMC_IT_TXUNDERR) != RESET)
    {
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_TXUNDERR);
        SDCardIO_DISABLE_IT(SDMMC_IT_TXUNDERR);
    	SDCardStatus = HAL_SD_STATE_READY;
        SDReadWriteError = TRUE;
    }
    else if(SDCardIO_GET_FLAG(SDMMC_IT_IDMATE) != RESET)
    {
        SDCardIO_TRANSFER_DISABLE();
        SDCardIO_CLEAR_FLAG(SDMMC_FLAG_IDMATE);
        SDCardIO_DISABLE_IT(SDMMC_IT_IDMATE);
       	SDCardStatus = HAL_SD_STATE_READY;
        SDReadWriteError = TRUE;
    }
}
#endif

/*
*   SD Card initialize 
*/
uint8_t SDCard_Init(void)
{
	uint32_t ret;

    if(SDCardInitDone)
    {
        printf("SD already init done!\r\n");
        return (SD_ERROR_NONE);
    }

    if(SDCardStatus == HAL_SD_STATE_RESET)
    {
        /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
        HAL_SD_MspInit(NULL);
    }

    SDCardStatus = HAL_SD_STATE_BUSY;

    ret = SD_InitProcess();
    if(ret != HAL_SD_ERROR_NONE)
    {
        printf("sd card init process error!, ret = %08x\r\n", ret);
        return (SD_ERROR_INITCARD);
    }

    ret = SD_EnableWideBus();
    if(ret != HAL_SD_ERROR_NONE)
    {
        printf("sd card config wide bus error!\r\n");
        return (SD_ERROR_INITCARD);
    }

	ret = SD_ConfigClock(4);
	if(ret != HAL_SD_ERROR_NONE)
    {
        printf("sd card config clock error!\r\n");
        return (SD_ERROR_INITCARD);
    }
    
    /* data for fatfs */
    SDCard_GetBaseInfo(&SDBaseInfo);

    SDCardContext = SD_CONTEXT_NONE;
    SDCardStatus = HAL_SD_STATE_READY;
    SDCardErrStatus = HAL_SD_ERROR_NONE;
    SDCardInitDone = TRUE;

    printf("SD init done\r\n");
    return (SD_ERROR_NONE);
}

uint8_t SDCard_GetCardState(void)
{
    HAL_SD_CardStateTypedef cardstate =  HAL_SD_CARD_TRANSFER;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    uint32_t resp1 = 0;
  
    errorstate = SD_GetCardStatus(&resp1);
  
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        SDCardErrStatus = errorstate;
        return (SD_ERROR);
    }
    /* Clear all the static flags */
    SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
    cardstate = (HAL_SD_CardStateTypedef)((resp1 >> 9) & 0x0F);
    //printf("GetCardState(), state = 0x%08x\r\n", cardstate);
    if(cardstate == HAL_SD_CARD_TRANSFER)
    {
        return (SD_READY);
    }
    else if((cardstate == HAL_SD_CARD_ERROR) || (cardstate == HAL_SD_CARD_DISCONNECTED))
    {
        return (SD_ERROR);
    }
    else
    {
        return (SD_BUSY);
    }
}

void SDCard_GetBaseInfo(SDBaseInfo_t *info)
{
    info->CardType      = SDCard.CardType;
    info->CardVersion   = SDCard.CardVersion;
    info->BlockNbr      = SDCard.BlockNbr;
    info->BlockSize     = SDCard.BlockSize;

    //printf("sdcard->type = %d\r\n", info->CardType);
	//printf("sdcard->version = %d\r\n", info->CardVersion);
	//printf("sdcard->blocknbr = %d\r\n", info->BlockNbr);
	//printf("sdcard->blocksize = %d\r\n", info->BlockSize);
}


/* SD Card read blocks */
uint8_t SDCard_ReadMultipleBlocks(uint8_t *pBuff, uint32_t BlockAddr, uint32_t BlockNbr)
{
    uint32_t errorstate;
    uint16_t timeout = 0xFFFF;
    volatile uint8_t result = SD_ERROR_PROC;
	
#if (SDCARD_DMA_MODE)
    /* SD Transfer DMA Mode
    */
    errorstate = SD_ReadBlocks_DMA(pBuff, BlockAddr, BlockNbr);
    //printf("SD_ReadBlocks_DMA(), err = 0x%08x, Addr = %d, Nbr = %d\r\n", errorstate, BlockAddr, BlockNbr);
    if(errorstate == HAL_SD_ERROR_NONE)
    {
        while(timeout--)
        {
            TimDelayMs(5);
            if(SDReadCompleted)
            {
                /* check SD Card state */
                errorstate = (uint32_t)SDCard_GetCardState();
                if(errorstate == SD_READY)
                {
                    result = SD_ERROR_NONE; 
                    break;
                }
            }
        
            if(SDReadWriteError == TRUE)
            {   
                result = SD_ERROR_PROC;
                break;
            }
        }
        /* SDMMC stop transfer */
        //SDCardIO_TRANSFER_DISABLE();
        /* Clear all the static flags */
        SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
        /* SDCARD stop transfer */
        if((SDCardContext & SD_CONTEXT_READ_MULTIPLE_BLOCK) != RESET)
        {
            SDMMC_CmdStopTransfer(SDCardIO);
        }
    }
#else
    /* SD Transfer Polling Mode
    */
    errorstate = SD_ReadBlocks(pBuff, BlockAddr, BlockNbr);
    //printf("SD_ReadBlocks(), err = 0x%08x, Addr = %d, Nbr = %d\r\n", errorstate, BlockAddr, BlockNbr);
    if(errorstate == HAL_SD_ERROR_NONE)
    {
        while(timeout--)
        {
            TimDelayMs(1);
            /* check SD Card state */
            errorstate = (uint32_t)SDCard_GetCardState();
            if(errorstate == SD_READY)
            {
                result = SD_ERROR_NONE;
                break;
            }
        }
    }   
    else
    {
    	result = SD_ERROR_PROC;
    	printf("SD_ReadBlocks(), err = 0x%08x, Addr = %d, Nbr = %d\r\n", errorstate, BlockAddr, BlockNbr);
    }
#endif    
	//TimDelayMs(10);
    return result;
}

/* SD Card write blocks */
uint8_t SDCard_WriteMultipleBlocks(uint8_t *pData, uint32_t BlockAddr, uint32_t BlockNbr)
{
    uint32_t errorstate;
    uint16_t timeout = 0xFFFF;
    volatile uint8_t result = SD_ERROR_PROC;
#if (SDCARD_DMA_MODE)
    errorstate = SD_WriteBlocks_DMA(pData, BlockAddr, BlockNbr);
    //printf("SD_WriteBlocks_DMA(), err = 0x%08x, Addr = %d, Nbr = %d\r\n", errorstate, BlockAddr, BlockNbr);
    if(errorstate == HAL_SD_ERROR_NONE)
    {
        while(timeout--)
        {
            TimDelayMs(5);
            if(SDWriteCompleted)
            {
                /* check SD Card state */
                errorstate = (uint32_t)SDCard_GetCardState();
                if(errorstate == SD_READY)
                {
                    result = SD_ERROR_NONE; 
                    break;
                }
            }

            if(SDReadWriteError == TRUE)
            {   
                result = SD_ERROR_PROC;
                break;
            }
        }

        /* SDMMC stop transfer */
        //SDCardIO_TRANSFER_DISABLE();
        /* Clear all the static flags */
        SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
        /* SDCARD stop transfer */
        if((SDCardContext & SD_CONTEXT_WRITE_MULTIPLE_BLOCK) != RESET)
        {
            SDMMC_CmdStopTransfer(SDCardIO);
        }
    }
#else
    errorstate = SD_WriteBlocks(pData, BlockAddr, BlockNbr);
    //printf("SD_WriteBlocks(), err = 0x%08x, Addr = %d, Nbr = %d\r\n", errorstate, BlockAddr, BlockNbr);
    if(errorstate == HAL_SD_ERROR_NONE)
    {
        while(timeout--)
        {
            TimDelayMs(1);
            /* check SD Card state */
            errorstate = (uint32_t)SDCard_GetCardState();
            if(errorstate == SD_READY)
            {
                result = SD_ERROR_NONE;
                break;
            }
        }
    }
#endif
	//TimDelayMs(10);
    return result;
}

/****************** PRIVATE FUNCTION **********************/
static uint32_t SD_InitProcess(void)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    SD_InitTypeDef Init;
  
    /* Default SDMMC peripheral configuration for SD card initialization */
    Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    Init.BusWide             = SDMMC_BUS_WIDE_1B;
    Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    Init.ClockDiv            = SDMMC_INIT_CLK_DIV;
    
    /* Initialize SDMMC peripheral interface with default configuration */
    SDMMC_Init(SDCardIO, Init);
  
    /* Set Power State to ON */
    SDMMC_PowerState_ON(SDCardIO);

    /* Identify card operating voltage */
    errorstate = SD_PowerON();
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        SDCardStatus = HAL_SD_STATE_READY;
        SDCardErrStatus = errorstate;
        printf("sd power on error\r\n");
        return errorstate;
    }

    /* Card initialization */
    errorstate = SD_InitCard();
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        SDCardStatus = HAL_SD_STATE_READY;
        SDCardErrStatus = errorstate;
        printf("sd card init error\r\n");
        return errorstate;
    }

    return (HAL_SD_ERROR_NONE);    
}

static uint32_t SD_PowerON(void)
{
    __IO uint32_t count = 0;
    uint32_t response = 0, validvoltage = 0;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
  
    /* CMD0: GO_IDLE_STATE */
    errorstate = SDMMC_CmdGoIdleState(SDCardIO);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        return errorstate;
    }
  
    /* CMD8: SEND_IF_COND: Command available only on V2.0 cards */
    errorstate = SDMMC_CmdOperCond(SDCardIO);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        //hsd->SdCard.CardVersion = CARD_V1_X;
        return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }
    else
    {
        SDCard.CardVersion = CARD_V2_X;
    }

    /* SEND CMD55 APP_CMD with RCA as 0 */
    errorstate = SDMMC_CmdAppCommand(SDCardIO, 0);
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }
    else
    {
        /* SD CARD */
        /* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
        while((!validvoltage) && (count < SDMMC_MAX_VOLT_TRIAL))
        {
            /* SEND CMD55 APP_CMD with RCA as 0 */
            errorstate = SDMMC_CmdAppCommand(SDCardIO, 0);
            if(errorstate != HAL_SD_ERROR_NONE)
            {
                return errorstate;
            }
      
            /* Send CMD41 */
            errorstate = SDMMC_CmdAppOperCommand(SDCardIO, SDMMC_VOLTAGE_WINDOW_SD | SDMMC_HIGH_CAPACITY | SD_SWITCH_1_8V_CAPACITY);
            if(errorstate != HAL_SD_ERROR_NONE)
            {
                return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
            }

            /* Get command response */
            response = SDMMC_GetResponse(SDCardIO, SDMMC_RESP1);

            /* Get operating voltage*/
            validvoltage = (((response >> 31) == 1) ? 1 : 0);

            count++;
        }

        if(count >= SDMMC_MAX_VOLT_TRIAL)
        {
            return HAL_SD_ERROR_INVALID_VOLTRANGE;
        }
    
        if((response & SDMMC_HIGH_CAPACITY) == SDMMC_HIGH_CAPACITY) /* (response &= SD_HIGH_CAPACITY) */
        {
            SDCard.CardType = CARD_SDHC_SDXC;
        }
        else
        {
            return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        }
    }

    return HAL_SD_ERROR_NONE;
}


static uint32_t SD_PowerOFF(void)
{
    return HAL_SD_ERROR_NONE;
}

static uint32_t SD_Reset(void)
{
    return HAL_SD_ERROR_NONE;
}

static uint32_t SD_GetCardCSD(HAL_SD_CardCSDTypedef *pCSD)
{
  uint32_t tmp = 0;
  
  /* Byte 0 */
  tmp = (SDCSD[0] & 0xFF000000U) >> 24;
  pCSD->CSDStruct      = (uint8_t)((tmp & 0xC0) >> 6);
  pCSD->SysSpecVersion = (uint8_t)((tmp & 0x3C) >> 2);
  pCSD->Reserved1      = tmp & 0x03;
  
  /* Byte 1 */
  tmp = (SDCSD[0] & 0x00FF0000) >> 16;
  pCSD->TAAC = (uint8_t)tmp;
  
  /* Byte 2 */
  tmp = (SDCSD[0] & 0x0000FF00) >> 8;
  pCSD->NSAC = (uint8_t)tmp;
  
  /* Byte 3 */
  tmp = SDCSD[0] & 0x000000FF;
  pCSD->MaxBusClkFrec = (uint8_t)tmp;
  
  /* Byte 4 */
  tmp = (SDCSD[1] & 0xFF000000U) >> 24;
  pCSD->CardComdClasses = (uint16_t)(tmp << 4);
  
  /* Byte 5 */
  tmp = (SDCSD[1] & 0x00FF0000U) >> 16;
  pCSD->CardComdClasses |= (uint16_t)((tmp & 0xF0) >> 4);
  pCSD->RdBlockLen       = (uint8_t)(tmp & 0x0F);
  
  /* Byte 6 */
  tmp = (SDCSD[1] & 0x0000FF00U) >> 8;
  pCSD->PartBlockRead   = (uint8_t)((tmp & 0x80) >> 7);
  pCSD->WrBlockMisalign = (uint8_t)((tmp & 0x40) >> 6);
  pCSD->RdBlockMisalign = (uint8_t)((tmp & 0x20) >> 5);
  pCSD->DSRImpl         = (uint8_t)((tmp & 0x10) >> 4);
  pCSD->Reserved2       = 0; /*!< Reserved */
        
  if(SDCard.CardType == CARD_SDSC)
  {
    pCSD->DeviceSize = (tmp & 0x03) << 10;
    
    /* Byte 7 */
    tmp = (uint8_t)(SDCSD[1] & 0x000000FFU);
    pCSD->DeviceSize |= (tmp) << 2;
    
    /* Byte 8 */
    tmp = (uint8_t)((SDCSD[2] & 0xFF000000U) >> 24);
    pCSD->DeviceSize |= (tmp & 0xC0) >> 6;
    
    pCSD->MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    pCSD->MaxRdCurrentVDDMax = (tmp & 0x07);
    
    /* Byte 9 */
    tmp = (uint8_t)((SDCSD[2] & 0x00FF0000U) >> 16);
    pCSD->MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    pCSD->MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    pCSD->DeviceSizeMul      = (tmp & 0x03) << 1;
    /* Byte 10 */
    tmp = (uint8_t)((SDCSD[2] & 0x0000FF00U) >> 8);
    pCSD->DeviceSizeMul |= (tmp & 0x80) >> 7;
    
    SDCard.BlockNbr  = (pCSD->DeviceSize + 1) ;
    SDCard.BlockNbr *= (1 << (pCSD->DeviceSizeMul + 2));
    SDCard.BlockSize = 1 << (pCSD->RdBlockLen);

    SDCard.LogBlockNbr =  (SDCard.BlockNbr) * ((SDCard.BlockSize) / 512); 
    SDCard.LogBlockSize = 512;
  }
  else if(SDCard.CardType == CARD_SDHC_SDXC)
  {
    /* Byte 7 */
    tmp = (uint8_t)(SDCSD[1] & 0x000000FFU);
    pCSD->DeviceSize = (tmp & 0x3F) << 16;
    
    /* Byte 8 */
    tmp = (uint8_t)((SDCSD[2] & 0xFF000000U) >> 24);
    
    pCSD->DeviceSize |= (tmp << 8);
    
    /* Byte 9 */
    tmp = (uint8_t)((SDCSD[2] & 0x00FF0000U) >> 16);
    
    pCSD->DeviceSize |= (tmp);
    
    /* Byte 10 */
    tmp = (uint8_t)((SDCSD[2] & 0x0000FF00U) >> 8);
    
    SDCard.LogBlockNbr = SDCard.BlockNbr = (((uint64_t)pCSD->DeviceSize + 1) * 1024);
    SDCard.LogBlockSize = SDCard.BlockSize = 512;
  }
  else
  {
    /* Clear all the static flags */
    SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
    SDCardErrStatus |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    SDCardStatus = HAL_SD_STATE_READY;
    return HAL_ERROR;
  }
  
  pCSD->EraseGrSize = (tmp & 0x40) >> 6;
  pCSD->EraseGrMul  = (tmp & 0x3F) << 1;
  
  /* Byte 11 */
  tmp = (uint8_t)(SDCSD[2] & 0x000000FF);
  pCSD->EraseGrMul     |= (tmp & 0x80) >> 7;
  pCSD->WrProtectGrSize = (tmp & 0x7F);
  
  /* Byte 12 */
  tmp = (uint8_t)((SDCSD[3] & 0xFF000000U) >> 24);
  pCSD->WrProtectGrEnable = (tmp & 0x80) >> 7;
  pCSD->ManDeflECC        = (tmp & 0x60) >> 5;
  pCSD->WrSpeedFact       = (tmp & 0x1C) >> 2;
  pCSD->MaxWrBlockLen     = (tmp & 0x03) << 2;
  
  /* Byte 13 */
  tmp = (uint8_t)((SDCSD[3] & 0x00FF0000) >> 16);
  pCSD->MaxWrBlockLen      |= (tmp & 0xC0) >> 6;
  pCSD->WriteBlockPaPartial = (tmp & 0x20) >> 5;
  pCSD->Reserved3           = 0;
  pCSD->ContentProtectAppli = (tmp & 0x01);
  
  /* Byte 14 */
  tmp = (uint8_t)((SDCSD[3] & 0x0000FF00) >> 8);
  pCSD->FileFormatGroup  = (tmp & 0x80) >> 7;
  pCSD->CopyFlag         = (tmp & 0x40) >> 6;
  pCSD->PermWrProtect    = (tmp & 0x20) >> 5;
  pCSD->TempWrProtect    = (tmp & 0x10) >> 4;
  pCSD->FileFormat       = (tmp & 0x0C) >> 2;
  pCSD->ECC              = (tmp & 0x03);
  
  /* Byte 15 */
  tmp = (uint8_t)(SDCSD[3] & 0x000000FF);
  pCSD->CSD_CRC   = (tmp & 0xFE) >> 1;
  pCSD->Reserved4 = 1;
 
  return HAL_OK;
}


static uint32_t SD_InitCard(void)
{
    HAL_SD_CardCSDTypedef CSD;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    uint16_t sd_rca = 1;
  
    /* Check the power State */
    if(SDMMC_GetPowerState(SDCardIO) == 0) 
    {
        /* Power off */
        return HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
    }
  
    if(SDCard.CardType != CARD_SECURED) 
    {
        /* Send CMD2 ALL_SEND_CID */
        errorstate = SDMMC_CmdSendCID(SDCardIO);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            return errorstate;
        }

        /* Get Card identification number data */
        //SDCID[0] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP1);
        //SDCID[1] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP2);
        //SDCID[2] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP3);
        //SDCID[3] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP4);
    }

    if(SDCard.CardType != CARD_SECURED) 
    {
        /* Send CMD3 SET_REL_ADDR with argument 0 */
        /* SD Card publishes its RCA. */
        errorstate = SDMMC_CmdSetRelAdd(SDCardIO, &sd_rca);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            return errorstate;
        }
    }
    
    if(SDCard.CardType != CARD_SECURED) 
    {
        /* Get the SD card RCA */
        SDCard.RelCardAdd = sd_rca;
  
        /* Send CMD9 SEND_CSD with argument as card's RCA */
        errorstate = SDMMC_CmdSendCSD(SDCardIO, (uint32_t)(SDCard.RelCardAdd << 16U));
        if(errorstate != HAL_SD_ERROR_NONE)     
        {
            return errorstate;
        }
        else
        {
            /* Get Card Specific Data */
            SDCSD[0U] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP1);
            SDCSD[1U] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP2);
            SDCSD[2U] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP3);
            SDCSD[3U] = SDMMC_GetResponse(SDCardIO, SDMMC_RESP4);
        }
    }
  
    /* Get the Card Class */
    SDCard.Class = (SDMMC_GetResponse(SDCardIO, SDMMC_RESP2) >> 20);
  
    /* Get CSD parameters */
    SD_GetCardCSD(&CSD);

    /* Select the Card */
    errorstate = SDMMC_CmdSelDesel(SDCardIO, (uint32_t)(((uint32_t)SDCard.RelCardAdd) << 16));
    if(errorstate != HAL_SD_ERROR_NONE)
    {
        return errorstate;
    }
  
    /* All cards are initialized */
    return HAL_SD_ERROR_NONE;
}

/* 4bits bus width
*/
static uint32_t SD_EnableWideBus(void)
{
    SDMMC_InitTypeDef Init;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    
    if((SDMMC_GetResponse(SDCardIO, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }   

    /* Send CMD55 APP_CMD with argument as card's RCA.*/
    errorstate = SDMMC_CmdAppCommand(SDCardIO, (uint32_t)(SDCard.RelCardAdd << 16));
    if(errorstate != HAL_OK)
    {
        return errorstate;
    }
  
    /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
    errorstate = SDMMC_CmdBusWidth(SDCardIO, 2);
    if(errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Configure the SDMMC peripheral */
    Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    Init.BusWide             = SDMMC_BUS_WIDE_4B;
    Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    //Init.ClockDiv            = SDMMC_NSpeed_CLK_DIV;//input 200mhz, need 25mhz
    SDMMC_Init(SDCardIO, Init);

    return (HAL_SD_ERROR_NONE);
}

/* 	SDMMC CLK = sdmmc_core_freq / (ClockDiv * 2)
*	@param ClockDiv: 0 ~ 1023
*/
static uint32_t SD_ConfigClock(uint8_t ClockDiv)
{
	SDMMC_InitTypeDef Init;

	/* Configure the SDMMC peripheral */
    Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    //Init.BusWide             = SDMMC_BUS_WIDE_4B;
    Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    Init.ClockDiv            = ClockDiv;
    
    return SDMMC_Init(SDCardIO, Init);
}

static uint32_t SD_DisableWideBus(void)
{
    SDMMC_InitTypeDef Init;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
  
    if((SDMMC_GetResponse(SDCardIO, SDMMC_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }

    /* Send CMD55 APP_CMD with argument as card's RCA */
    errorstate = SDMMC_CmdAppCommand(SDCardIO, (uint32_t)(SDCard.RelCardAdd << 16));
    if(errorstate != HAL_OK)
    {
        return errorstate;
    }
  
    /* Send ACMD6 APP_CMD with argument as 0 for single bus mode */
    errorstate = SDMMC_CmdBusWidth(SDCardIO, 0);
    if(errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Configure the SDMMC peripheral */
    Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
    Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    Init.BusWide             = SDMMC_BUS_WIDE_1B;
    Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    Init.ClockDiv            = SDMMC_NSpeed_CLK_DIV;//input 200mhz, need 25mhz
    SDMMC_Init(SDCardIO, Init);

    return (HAL_SD_ERROR_NONE);
}

static uint32_t SD_GetCardStatus(uint32_t *pCardStatus)
{
  uint32_t errorstate = HAL_SD_ERROR_NONE;
  
  if(pCardStatus == NULL)
  {
    return HAL_SD_ERROR_PARAM;
  }
  
  /* Send Status command */
  errorstate = SDMMC_CmdSendStatus(SDCardIO, (uint32_t)(SDCard.RelCardAdd << 16));
  if(errorstate != HAL_OK)
  {
    return errorstate;
  }
  
  /* Get SD card status */
  *pCardStatus = SDMMC_GetResponse(SDCardIO, SDMMC_RESP1);
  
  return HAL_SD_ERROR_NONE;
}


static uint32_t SD_ReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    uint32_t count = 0, timeout = 0xffff; 
    uint32_t *tempbuff = (uint32_t *)pData;
  
    if(NULL == pData)
    {
        SDCardErrStatus = HAL_SD_ERROR_PARAM;
        return (SDCardErrStatus);
    }
 
    if(SDCardStatus == HAL_SD_STATE_READY)
    {
        SDCardErrStatus = HAL_SD_ERROR_NONE;
    
        if((BlockAdd + NumberOfBlocks) > (SDCard.LogBlockNbr))
        {
            SDCardErrStatus = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return (SDCardErrStatus);
        }

        /* check current card state */
        if(SDCard_GetCardState() != SD_READY)
        {
            SDCardErrStatus = HAL_SD_ERROR_BUSY;
            return (SDCardErrStatus);
        }
    
        SDCardStatus = HAL_SD_STATE_BUSY;
    
        /* Initialize data control register */
        SDCardIO->DCTRL = 0;
    
        if(SDCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512;
        }
      
        /* Set Block Size for Card */
        errorstate = SDMMC_CmdBlockLength(SDCardIO, BLOCKSIZE);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
    
        /* Read block(s) in polling mode */
        if(NumberOfBlocks > 1)
        {
            SDCardContext = SD_CONTEXT_READ_MULTIPLE_BLOCK;
      
            /* Read Multi Block command */ 
            errorstate = SDMMC_CmdReadMultiBlock(SDCardIO, BlockAdd);
        }
        else
        {
            SDCardContext = SD_CONTEXT_READ_SINGLE_BLOCK;
      
            /* Read Single Block command */
            errorstate = SDMMC_CmdReadSingleBlock(SDCardIO, BlockAdd);
        }
        /* cmd error state */
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }

        /* Configure the SD DPSM (Data Path State Machine) */
        config.DataTimeOut   = SDMMC_DATATIMEOUT;
        config.DataLength    = NumberOfBlocks * BLOCKSIZE;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
        config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
        config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
        config.DPSM          = SDMMC_DPSM_ENABLE;
        SDMMC_ConfigData(SDCardIO, &config);
        /* reading state */
        //SDCardIO_TRANSFER_ENABLE();
        SDCardStatus = HAL_SD_STATE_RECEIVING;
        /* Poll on SDMMC flags */
        while(!SDCardIO_GET_FLAG(SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
        {
            if (SDCardIO_GET_FLAG(SDMMC_FLAG_RXFIFOHF))
            {
                /* Read data from SDMMC Rx FIFO */
                for(count = 0U; count < 8U; count++)
                {
                    *(tempbuff + count) = SDMMC_ReadFIFO(SDCardIO);
                }
                tempbuff += 8U;
                timeout = 0xffff;
            }
            //DelayUs(50);
            if((timeout == 0) || (--timeout == 0))
            {
                /* Clear all the static flags */
                SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
                SDCardErrStatus = HAL_SD_ERROR_TIMEOUT;
                SDCardStatus = HAL_SD_STATE_READY;
                return (SDCardErrStatus);
            }
        }
    
        /* Send stop transmission command in case of multiblock read */
        if(SDCardIO_GET_FLAG(SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
        {    
            if(SDCard.CardType != CARD_SECURED)
            {
                /* Send stop transmission command */
                errorstate = SDMMC_CmdStopTransfer(SDCardIO);
                if(errorstate != HAL_SD_ERROR_NONE)
                {
                    /* Clear all the static flags */
                    SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
                    SDCardErrStatus = errorstate;
                    SDCardStatus = HAL_SD_STATE_READY;
                    return (SDCardErrStatus);
                }
            }
        }
    
        /* Get error state */
        if (SDCardIO_GET_FLAG(SDMMC_FLAG_DTIMEOUT))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_DATA_TIMEOUT;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
        else if (SDCardIO_GET_FLAG(SDMMC_FLAG_DCRCFAIL))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_DATA_CRC_FAIL;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
        else if (SDCardIO_GET_FLAG(SDMMC_FLAG_RXOVERR))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_RX_OVERRUN;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
    
        /* Clear all the static flags */
        SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
        SDCardStatus = HAL_SD_STATE_READY;
    
        return (HAL_SD_ERROR_NONE);
    }
    else
    {
        SDCardErrStatus = HAL_SD_ERROR_BUSY;
        return (SDCardErrStatus);
    }
}

static uint32_t SD_WriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
    uint32_t count = 0, timeout = 0xffff;
    uint32_t *tempbuff = (uint32_t *)pData;
  
    if(NULL == pData)
    {
        SDCardErrStatus = HAL_SD_ERROR_PARAM;
        return (SDCardErrStatus);
    }

    if(SDCardStatus == HAL_SD_STATE_READY)
    {
        SDCardErrStatus = HAL_SD_ERROR_NONE;
    
        if((BlockAdd + NumberOfBlocks) > (SDCard.LogBlockNbr))
        {
            SDCardErrStatus = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return (SDCardErrStatus);
        }

        /* check current card state */
        if(SDCard_GetCardState() != SD_READY)
        {
            SDCardErrStatus = HAL_SD_ERROR_BUSY;
            return (SDCardErrStatus);
        }
    
        SDCardStatus = HAL_SD_STATE_BUSY;
    
        /* Initialize data control register */
        SDCardIO->DCTRL = 0;
     
        if(SDCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512;
        }
    
        /* Set Block Size for Card */ 
        errorstate = SDMMC_CmdBlockLength(SDCardIO, BLOCKSIZE);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);  
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }

		/* Configure the SD DPSM (Data Path State Machine) */
		config.DataTimeOut	 = SDMMC_DATATIMEOUT;
		config.DataLength	 = NumberOfBlocks * BLOCKSIZE;
		config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
		config.TransferDir	 = SDMMC_TRANSFER_DIR_TO_CARD;
		config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
		config.DPSM 		 = SDMMC_DPSM_ENABLE;
		SDMMC_ConfigData(SDCardIO, &config);

        /* Write Blocks in Polling mode */
		if(NumberOfBlocks > 1U)
		{
			SDCardContext = SD_CONTEXT_WRITE_MULTIPLE_BLOCK;
			  
			/* Write Multi Block command */ 
			errorstate = SDMMC_CmdWriteMultiBlock(SDCardIO, BlockAdd);
		}
		else
		{
			SDCardContext = SD_CONTEXT_WRITE_SINGLE_BLOCK;
			  
			/* Write Single Block command */
			errorstate = SDMMC_CmdWriteSingleBlock(SDCardIO, BlockAdd);
		}
		/* error state */
		if(errorstate != HAL_SD_ERROR_NONE)
		{
			/* Clear all the static flags */
			SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
			SDCardErrStatus = errorstate;
			SDCardStatus = HAL_SD_STATE_READY;
			return (SDCardErrStatus);
		}
        
        /* reading state */
        //SDCardIO_TRANSFER_ENABLE();
        SDCardStatus = HAL_SD_STATE_TRANSFER;
    
        /* Write block(s) in polling mode */
        while(!SDCardIO_GET_FLAG(SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
        {
            if (SDCardIO_GET_FLAG(SDMMC_FLAG_TXFIFOHE))
            {
            	timeout = 0xffff;
                /* Write data to SDMMC Tx FIFO */
                for(count = 0U; count < 8U; count++)
                {
                    SDMMC_WriteFIFO(SDCardIO, (tempbuff + count));
                }
                tempbuff += 8U;
            }
            DelayUs(1);
            if((timeout == 0)||(--timeout == 0))
            {
                /* Clear all the static flags */
                SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);  
                SDCardErrStatus = errorstate;
                SDCardStatus = HAL_SD_STATE_READY;
                printf("WriteBlocks timeout\r\n");
                return (SDCardErrStatus);
            }
        }
    
        /* Send stop transmission command in case of multiblock write */
        if(SDCardIO_GET_FLAG(SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1U))
        { 
            if(SDCard.CardType != CARD_SECURED)
            { 
                /* Send stop transmission command */
                errorstate = SDMMC_CmdStopTransfer(SDCardIO);
                if(errorstate != HAL_SD_ERROR_NONE)
                {
                    /* Clear all the static flags */
                    SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);  
                    SDCardErrStatus = errorstate;
                    SDCardStatus = HAL_SD_STATE_READY;
                    return (SDCardErrStatus);
                }
            }
        }
    
        /* Get error state */
        if (SDCardIO_GET_FLAG(SDMMC_FLAG_DTIMEOUT))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_DATA_TIMEOUT;
            SDCardStatus = HAL_SD_STATE_READY;
            printf("WriteBlocks dtimeout\r\n");
            return (SDCardErrStatus);
        }
        else if (SDCardIO_GET_FLAG(SDMMC_FLAG_DCRCFAIL))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_DATA_CRC_FAIL;
            SDCardStatus = HAL_SD_STATE_READY;
            printf("WriteBlocks dcrc fail\r\n");
            return (SDCardErrStatus);
        }
        else if (SDCardIO_GET_FLAG(SDMMC_FLAG_TXUNDERR))
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardErrStatus = HAL_SD_ERROR_TX_UNDERRUN;
            SDCardStatus = HAL_SD_STATE_READY;
            printf("WriteBlocks tx fifo under run\r\n");
            return (SDCardErrStatus);
        }
    
        /* Clear all the static flags */
        SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
    
        SDCardStatus = HAL_SD_STATE_READY;
    
        return (HAL_SD_ERROR_NONE);
    }
    else
    {
        SDCardErrStatus = HAL_SD_ERROR_BUSY;
        return (SDCardErrStatus);
    }
}


#if (SDCARD_DMA_MODE)
static uint32_t SD_ReadBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    if(NULL == pData)
    {
        SDCardErrStatus = HAL_SD_ERROR_PARAM;
        return (SDCardErrStatus);
    }
  
    if(SDCardStatus == HAL_SD_STATE_READY)
    {
        SDCardErrStatus = HAL_SD_ERROR_NONE;
    
        if((BlockAdd + NumberOfBlocks) > (SDCard.LogBlockNbr))
        {
            SDCardErrStatus = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return (SDCardErrStatus);
        }

        /* check current card state */
        if(SDCard_GetCardState() != SD_READY)
        {
            SDCardErrStatus = HAL_SD_ERROR_BUSY;
            return (SDCardErrStatus);
        }
    
        SDCardStatus = HAL_SD_STATE_BUSY;
    
        /* Initialize data control register */
        SDCardIO->DCTRL = 0U;
    
        pRxBuffPtr = (uint32_t*)pData;
        RxBuffSize = BLOCKSIZE * NumberOfBlocks;
        
        if(SDCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        /* Set Block Size for Card */ 
        errorstate = SDMMC_CmdBlockLength(SDCardIO, BLOCKSIZE);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }

        /* Read Blocks in DMA mode */
        if(NumberOfBlocks > 1U)
        {
            SDCardContext = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_DMA);
      
            /* Read Multi Block command */
            errorstate = SDMMC_CmdReadMultiBlock(SDCardIO, BlockAdd);
        }
        else
        {
            SDCardContext = (SD_CONTEXT_READ_SINGLE_BLOCK | SD_CONTEXT_DMA);
      
            /* Read Single Block command */
            errorstate = SDMMC_CmdReadSingleBlock(SDCardIO, BlockAdd);
        }
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS);
            SDCardIO_DISABLE_IT(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR \
                                | SDMMC_IT_DATAEND | SDMMC_IT_IDMATE);
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
    	/* Enable transfer interrupts */
        SDCardIO_ENABLE_IT(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_RXOVERR \
                            | SDMMC_IT_DATAEND | SDMMC_IT_IDMATE);
        /* SDMMC DMA config */
        SDCardIO->IDMACTRL  = SDMMC_ENABLE_IDMA_SINGLE_BUFF; 
        SDCardIO->IDMABASE0 = (uint32_t) pData ;        
        
        /* Configure the SD DPSM (Data Path State Machine) */ 
        config.DataTimeOut   = SDMMC_DATATIMEOUT;
        config.DataLength    = BLOCKSIZE * NumberOfBlocks;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
        config.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
        config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
        config.DPSM          = SDMMC_DPSM_ENABLE;
        SDMMC_ConfigData(SDCardIO, &config);
        /* transfer enable */
        //SDCardIO_TRANSFER_ENABLE();
        /* reading state */
        SDCardStatus = HAL_SD_STATE_RECEIVING;
        SDReadCompleted = FALSE;
        SDReadWriteError = FALSE;        
    
        return (HAL_SD_ERROR_NONE);
    }
    else
    {
        SDCardErrStatus = HAL_SD_ERROR_BUSY;
        return (SDCardErrStatus);
    }
}

static uint32_t SD_WriteBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDMMC_DataInitTypeDef config;
    uint32_t errorstate = HAL_SD_ERROR_NONE;
  
    if(NULL == pData)
    {
        SDCardErrStatus = HAL_SD_ERROR_PARAM;
        return (SDCardErrStatus);
    }

    if(SDCardStatus == HAL_SD_STATE_READY)
    {
        SDCardErrStatus = HAL_SD_ERROR_NONE;
    
        if((BlockAdd + NumberOfBlocks) > (SDCard.LogBlockNbr))
        {
            SDCardErrStatus = HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            return (SDCardErrStatus);
        }

        /* check current card state */
        if(SDCard_GetCardState() != SD_READY)
        {
            SDCardErrStatus = HAL_SD_ERROR_BUSY;
            return (SDCardErrStatus);
        }
    
        SDCardStatus = HAL_SD_STATE_BUSY;
    
        /* Initialize data control register */
        SDCardIO->DCTRL = 0U;
    
        pTxBuffPtr = (uint32_t*)pData;
        TxBuffSize = BLOCKSIZE * NumberOfBlocks;

        if(SDCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        /* Set Block Size for Card */ 
        errorstate = SDMMC_CmdBlockLength(SDCardIO, BLOCKSIZE);
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
            SDCardErrStatus = errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }
        /* Write Blocks in Polling mode */
        if(NumberOfBlocks > 1U)
        {
            SDCardContext = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_DMA);
      
            /* Write Multi Block command */ 
            errorstate = SDMMC_CmdWriteMultiBlock(SDCardIO, BlockAdd);
        }
        else
        {
            SDCardContext = (SD_CONTEXT_WRITE_SINGLE_BLOCK | SD_CONTEXT_DMA);
      
            /* Write Single Block command */
            errorstate = SDMMC_CmdWriteSingleBlock(SDCardIO, BlockAdd);
        }
        if(errorstate != HAL_SD_ERROR_NONE)
        {
            /* Clear all the static flags */
            SDCardIO_CLEAR_FLAG(SDMMC_STATIC_FLAGS); 
            SDCardIO_DISABLE_IT(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR \
                                | SDMMC_IT_DATAEND | SDMMC_IT_IDMATE); 
            SDCardErrStatus |= errorstate;
            SDCardStatus = HAL_SD_STATE_READY;
            return (SDCardErrStatus);
        }

		/* Enable transfer interrupts */
		SDCardIO_ENABLE_IT(SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_TXUNDERR \
									| SDMMC_IT_DATAEND | SDMMC_IT_IDMATE); 
		/* SDMMC DMA config */
		SDCardIO->IDMACTRL	= SDMMC_ENABLE_IDMA_SINGLE_BUFF; 
		SDCardIO->IDMABASE0 = (uint32_t) pData ;
        
        /* Configure the SD DPSM (Data Path State Machine) */
        config.DataTimeOut   = SDMMC_DATATIMEOUT;
        config.DataLength    = BLOCKSIZE * NumberOfBlocks;
        config.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
        config.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
        config.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
        config.DPSM          = SDMMC_DPSM_ENABLE;
        SDMMC_ConfigData(SDCardIO, &config);
        /* Transfer enable */
		//SDCardIO_TRANSFER_ENABLE(); 
        /* reading state */
        SDCardStatus = HAL_SD_STATE_TRANSFER;
        SDWriteCompleted = FALSE;
        SDReadWriteError = FALSE;
               
  
        return (HAL_SD_ERROR_NONE);
    }
    else
    {
        SDCardStatus = HAL_SD_ERROR_BUSY;
        return (SDCardStatus);
    }
}

#endif

#if 0
static uint32_t SD_Read_IT(void)
{
  uint32_t count = 0;
  uint32_t* tmp;

  tmp = (uint32_t*)pRxBuffPtr;
  
  /* Read data from SDMMC Rx FIFO */
  for(count = 0; count < 8; count++)
  {
    *(tmp + count) = SDMMC_ReadFIFO(SDCardIO);
  }
  pRxBuffPtr += 8;
  
  return (HAL_SD_ERROR_NONE);
}

static uint32_t SD_Write_IT(void)
{
    uint32_t count = 0;
    uint32_t* tmp;
  
    tmp = (uint32_t*)pTxBuffPtr;
  
    /* Write data to SDMMC Tx FIFO */
    for(count = 0; count < 8; count++)
    {
        SDMMC_WriteFIFO(SDCardIO, (tmp + count));
    }
    pTxBuffPtr += 8;

    return (HAL_SD_ERROR_NONE);
}
#endif



