
#include "stm32h7xx.h"
#include "uart_atk.h"

/***********************/
//#define ATKUART_RXBUFF_SIZE     (2560U)
#define UART_IT_RTOIE			((uint32_t)1<<26)
#define UART_IT_RTOEN			((uint32_t)1<<23)

/*  STATIC
*/
static void USART_Receive_IT(UART_HandleTypeDef *huart);
static void USART_Transmit_IT(UART_HandleTypeDef *huart);

/* ATK Uart callback */
static ATK_RxParserCallback ATK_RxParserFunc = NULL;
static ATK_RxCpltCallback ATK_RxCpltProcFunc = NULL;
static ATK_TxCpltCallback ATK_TxCpltProcFunc = NULL;
//static ATK_ErrorCallback ATK_ErrorProcFunc = NULL;
/* hal handler */
static UART_HandleTypeDef ATKUart_Handle;
/* rx&tx control */
static uint8_t ATK_UartRxBuffer[8];
__IO bool ATK_UartRxCplt = FALSE;
__IO bool ATK_UartRxEnable = FALSE;
__IO bool ATK_UartTxCplt = FALSE;
__IO bool ATK_UartTxEnable = FALSE;


/********** PUBLIC FUNCTION ************/
uint8_t ATKUart_Init(UartDevice_t *dev)
{
    GPIO_InitTypeDef GPIO_Init;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
	/* GPIO Init */
    GPIO_Init.Pin = GPIO_PIN_10 | GPIO_PIN_11;		
    GPIO_Init.Mode = GPIO_MODE_AF_PP;		
    GPIO_Init.Pull = GPIO_PULLUP;			
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Alternate = GPIO_AF7_USART3;	
    HAL_GPIO_Init(GPIOB, &GPIO_Init);	   		 
    /* UART Init */
	ATKUart_Handle.Instance = USART3;					
	ATKUart_Handle.Init.BaudRate = dev->baudrate;				  
	ATKUart_Handle.Init.WordLength = dev->wordlength;  
	ATKUart_Handle.Init.StopBits = dev->stopbits;	   
	ATKUart_Handle.Init.Parity = dev->parity;		  
	ATKUart_Handle.Init.Mode = dev->mode;
	ATKUart_Handle.Init.HwFlowCtl = dev->hwflowctrl;
	ATKUart_Handle.Init.Prescaler = UART_PRESCALER_DIV16;
	//ATKUart_Handle.Init.FIFOMode = UART_FIFOMODE_ENABLE;
	//ATKUart_Handle.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_2;
	//ATKUart_Handle.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_2;
	if(HAL_UART_Init(&ATKUart_Handle) != HAL_OK)
	{
	    return (1);//ERROR
	}

    /* IRQ */
    HAL_NVIC_SetPriority(USART3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    /* callback */
    ATK_RxParserFunc = dev->rxParserFunc;
    ATK_RxCpltProcFunc = dev->rxCpltProcFunc;
    ATK_TxCpltProcFunc = dev->txCpltProcFunc;
    //ATK_ErrorProcFunc = dev->errorProcFunc;
    /* enable receiver */
    if(dev->mode & USART_MODE_RX)
    {
        //SET_BIT(ATKUart_Handle.Instance->CR1, USART_CR1_RTOIE);
        ATKUart_ReceiveIT(ATK_UartRxBuffer, 8);
	}
	
    return (0);//ERROR NONE
}

void ATKUart_DeInit(UartDevice_t *dev)
{
	(void)dev;

	__HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_USART3_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(USART3_IRQn);
	HAL_UART_DeInit(&ATKUart_Handle);

	ATK_RxParserFunc = NULL;
    ATK_RxCpltProcFunc = NULL;
    ATK_TxCpltProcFunc = NULL;
    //ATK_ErrorProcFunc = NULL;
}


void ATKUart_TransmitIT(uint8_t *pData, uint16_t size)
{
	ATK_UartTxEnable = TRUE;

	__HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_TXE|UART_FLAG_TC);
    HAL_UART_Transmit_IT(&ATKUart_Handle, pData, size);
}

void ATKUart_ReceiveIT(uint8_t *pData, uint16_t size)
{
	ATK_UartRxEnable = TRUE;
    /* clear flag */
	__HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_RXNE | UART_FLAG_RTOF);
	/* enable receiver */
    HAL_UART_Receive_IT(&ATKUart_Handle, pData, size);
    /* setup receiver timeout */
    MODIFY_REG(ATKUart_Handle.Instance->RTOR, 0x00ffffff, 0xffff);
    SET_BIT(ATKUart_Handle.Instance->CR1, UART_IT_RTOIE);
  	SET_BIT(ATKUart_Handle.Instance->CR2, UART_IT_RTOEN);
}

/* STM32 USART3 IRQ */
#if 0
void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&ATKUart_Handle);
}
#else
void USART3_IRQHandler(void)
{
    volatile uint16_t RDR = 0;
    uint32_t isrflags   = READ_REG(ATKUart_Handle.Instance->ISR);
    uint32_t cr1its     = READ_REG(ATKUart_Handle.Instance->CR1);
    uint32_t cr3its     = READ_REG(ATKUart_Handle.Instance->CR3);
    uint32_t errorflags = RESET;
    
     /* If no error occurs */
    //errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
    if (errorflags == RESET)
    {
        /* UART in mode Receiver ---------------------------------------------------*/
        if(ATK_UartRxEnable)
        {
        	/* read RDR */
        	if(__HAL_UART_GET_FLAG(&ATKUart_Handle, UART_FLAG_RXNE) != RESET)
         	{
             	__HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_RXNE);
             	
              	ATKUart_Handle.gState = HAL_UART_STATE_BUSY_TX_RX;
            	//USART_Receive_IT();
              	RDR = ATKUart_Handle.Instance->RDR;
             	/* data parser */
             	if(ATK_RxParserFunc)
              	{
                  	ATK_RxParserFunc((uint8_t)RDR);
             	}                    
         	}
                
       		/* timeout */
          	if(__HAL_UART_GET_FLAG(&ATKUart_Handle, UART_FLAG_RTOF) != RESET)
	     	{
	           	__HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_RTOF);
	            	
	          	ATKUart_Handle.gState = HAL_UART_STATE_READY;
	         	ATKUart_Handle.ErrorCode = HAL_UART_ERROR_NONE;
	         	/* rx compeleted */
	          	if(ATK_RxCpltProcFunc)
	         	{
	              	ATK_RxCpltProcFunc();
	         	}
	    	}
        }

        /* UART in mode Transmit ---------------------------------------------------*/
        if(ATK_UartTxEnable)
        {
            if(__HAL_UART_GET_FLAG(&ATKUart_Handle, UART_FLAG_TXE) != RESET)
            {
                USART_Transmit_IT(&ATKUart_Handle);
                __HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_TXE);
                
                ATKUart_Handle.gState = HAL_UART_STATE_BUSY_TX_RX;
                //return;
            }

            if((isrflags & USART_ISR_TC) && (cr1its & USART_CR1_TCIE))
            {
                __HAL_UART_CLEAR_FLAG(&ATKUart_Handle, UART_FLAG_TC);
                /* Disable the UART Transmit Complete Interrupt */
                CLEAR_BIT(ATKUart_Handle.Instance->CR1, USART_CR1_TCIE);
                ATKUart_Handle.gState = HAL_UART_STATE_READY;
                ATKUart_Handle.ErrorCode = HAL_UART_ERROR_NONE;
                ATK_UartTxEnable = FALSE;
                if(ATK_TxCpltProcFunc)
                {
                    ATK_TxCpltProcFunc();
                }
            }
        }
    }
}
#endif

/********** PRIVATE FUNCTION ************/
static void USART_Receive_IT(UART_HandleTypeDef *huart)
{
    uint16_t* tmp;
    uint16_t  uhMask = huart->Mask;
    uint16_t  uhdata;

    /* Check that a Rx process is ongoing */
    uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
        tmp = (uint16_t*) huart->pRxBuffPtr ;
        *tmp = (uint16_t)(uhdata & uhMask);
        huart->pRxBuffPtr +=2;
    }
    else
    {
        *huart->pRxBuffPtr++ = (uint8_t)(uhdata & (uint8_t)uhMask);
    }
}

static void USART_Transmit_IT(UART_HandleTypeDef *huart)
{
    uint16_t* tmp;

    /* Check that a Tx process is ongoing */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
        tmp = (uint16_t*) huart->pTxBuffPtr;
        huart->Instance->TDR = (*tmp & (uint16_t)0x01FFU);
        huart->pTxBuffPtr += 2U;
    }
    else
    {
        huart->Instance->TDR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0xFFU);
    }
    huart->TxXferCount--;
    
    if(huart->TxXferCount == 0U)
    {
        /* Disable the TX FIFO threshold interrupt (if FIFO mode is enabled) or
            Transmit Data Register Empty interrupt (if FIFO mode is Disabled).
        */
        if (READ_BIT(huart->Instance->CR1, USART_CR1_FIFOEN) != RESET)
        {
            CLEAR_BIT(huart->Instance->CR3, USART_CR3_TXFTIE);
        }
        else
        {
            CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);
        }
        /* Enable the UART Transmit Complete Interrupt */
        SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
    }
}

