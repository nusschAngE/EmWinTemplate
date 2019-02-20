
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "stm32h7xx.h"
#include "wifi_esp8266.h"
#include "uart_atk.h"

#include "my_malloc.h"
#include "delay.h"

/********* DEFINES ********************/
#define ESP_REST_PORT		GPIOI
#define ESP_REST_PIN		GPIO_PIN_11
#define ESP_REST_ON()		HAL_GPIO_WritePin(ESP_REST_PORT, ESP_REST_PIN, GPIO_PIN_RESET)
#define ESP_REST_OFF()		HAL_GPIO_WritePin(ESP_REST_PORT, ESP_REST_PIN, GPIO_PIN_SET)


/* pass through packet size */
#define ESP_PACKET_SIZE     (2048U)
#define ESP_CMDBUFF_SIZE    (512U)
#define ESP_DATABUFF_SIZE   (2048U + 32U)

//#define ESP_WaitCondition   WaitCondition
#define ESP_WaitCondition   OSWaitCondition

/*  STATIC
*/
static void esp_IoInit(void);
static bool esp_GetTxCplt(void);
static bool esp_GetRxCplt(void);
static void esp_SendData(uint8_t *pData, uint16_t size);
static bool esp_SendATCmd(const char *atcmd, ...);
static uint8_t esp_WaitResponse(uint8_t *param, uint16_t pSize, const char *response, uint32_t timeout);
static void esp_RxParser(uint8_t data);
static void esp_RxCpltProc(void);
static void esp_TxCpltProc(void);
static void esp_ErrorProc(void);
static bool esp_MessagePoolPut(EspMesg_t *pool, uint8_t *data, uint32_t size);
static uint32_t esp_MessagePoolGet(EspMesg_t *pool, uint8_t *buffer);

/********* VALUE ***********************/
static UartDevice_t EspUartDev = 
{
    115200,
    UART_WORDLENGTH_8B,
    UART_STOPBITS_1,
    UART_PARITY_NONE,
    UART_MODE_TX_RX,
    UART_HWCONTROL_NONE,
    esp_RxParser,
    esp_RxCpltProc,
    esp_TxCpltProc,
    esp_ErrorProc,
};

static EspMesg_t EspSendMesgPool[ESP_MAX_MESG], EspRecvMesgPool[ESP_MAX_MESG];
static uint8_t EspCmdBuffer[ESP_CMDBUFF_SIZE];
static uint8_t EspDataBuffer[ESP_DATABUFF_SIZE];
static __IO uint16_t EspRxDataSize = 0;
static __IO uint16_t EspRxByteIdx = 0;
static __IO uint8_t IsBufferBusy = 0;
static __IO uint8_t IsEspRxCplt = 0;
static __IO uint8_t IsEspTxCplt = 0;

/* IP Connect manage */
//static IPConnect_t ipConnected[4];

/********* PUBLIC FUNCTION ***************/
uint8_t ESP8266_Init(void)
{   
    uint8_t ret = 0;
    /* IO init */
    esp_IoInit();
    /* hard reset */
	ESP_REST_ON();
	TimDelayMs(200);
	ESP_REST_OFF();
	TimDelayMs(1500);//skip boot message
	
    /* uart init */
    ret = ATKUart_Init(&EspUartDev);
    if(ret != 0) {return (ESP_RES_ERROR);}  

    /* value init */
    myMemset(EspCmdBuffer, 0, ESP_CMDBUFF_SIZE);
    myMemset(EspDataBuffer, 0, ESP_DATABUFF_SIZE);
    myMemset(EspSendMesgPool, 0, ESP_MAX_MESG*sizeof(EspSendMesgPool[0]));
    myMemset(EspRecvMesgPool, 0, ESP_MAX_MESG*sizeof(EspSendMesgPool[0]));
    EspRxDataSize = 0;
    EspRxByteIdx = 0;
    IsBufferBusy = 0;
    IsEspRxCplt = ESP_XSFER_FREE;
    IsEspTxCplt = ESP_XSFER_FREE;

    return (ESP_RES_OK);
}

void ESP8266_DeInit(void)
{
    /* uart deinit */
    ATKUart_DeInit(&EspUartDev);

    /* value init */
    myMemset(EspCmdBuffer, 0, ESP_CMDBUFF_SIZE);
    myMemset(EspDataBuffer, 0, ESP_DATABUFF_SIZE);
    myMemset(EspSendMesgPool, 0, ESP_MAX_MESG*sizeof(EspSendMesgPool[0]));
    myMemset(EspRecvMesgPool, 0, ESP_MAX_MESG*sizeof(EspSendMesgPool[0]));
    EspRxDataSize = 0;
    EspRxByteIdx = 0;
    IsBufferBusy = 0;
    IsEspRxCplt = ESP_XSFER_FREE;
    IsEspTxCplt = ESP_XSFER_FREE;
}

uint8_t ESP8266_UartConfig(uint8_t baudRate, uint8_t bits, uint8_t stopbits, uint8_t pority, uint8_t flowc)
{
    /* module uart config */

    /* mcu uart config */
	
    return (0);
}

uint8_t ESP8266_TestATCmd(void)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_TEST);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//ok
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);    
}

/*  0:off, 1:on
*/
uint8_t ESP8266_EchoOnoff(uint8_t on)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_ATE, on);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//ok
        return (ESP_RES_TIMEOUT);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);
}

uint8_t ESP8266_Reset(void)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_RESET);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) 
    {
        TimDelayMs(3000);
        myMemset(EspDataBuffer, 0, EspRxDataSize);
	    EspDataBuffer[0] = 0;
	    IsBufferBusy = FALSE; 
	    IsEspRxCplt = ESP_XSFER_FREE;
	    EspRxDataSize = 0;
	    EspRxByteIdx = 0;

	    return (ESP_RES_OK);
    }
    else if(ret == 2) {
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);
}

/*  1:station, 2:ap, 3:ap+station
*   after this, need send cmd'AT+RST\r\n' to restart
*/
uint8_t ESP8266_SetWlanMode(uint8_t mode)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_SCWMODE, mode);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//ok
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);    
}

uint8_t ESP8266_JoinAP(uint8_t *ssid, uint8_t *pwd)
{
    uint8_t ret = 0, waitTimes = 5;

    esp_SendATCmd(CMD_SCWJAP, ssid, pwd);
    /* maybe it receive "CONNECTED" after "DISCONNECT" */
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, "CONNECTED|ERROR", 8000);
        if(ret == 1) {//ok
            return (ESP_RES_OK);
        } else if(ret == 2) {//error
            return (ESP_RES_ERROR);
        }
    }

    return (ESP_RES_TIMEOUT); 
}


uint8_t ESP8266_QuitAP(void)
{
    uint8_t ret = 0, waitTimes = 5;

    esp_SendATCmd(CMD_PCWQAP);
    /* wait DISCONNECT */
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, "DISCONNECTED|OK|ERROR", 1000);
        if((ret == 1) || (ret == 2)) {//ok
            return (ESP_RES_OK);
        } else if(ret == 3) {//error
            return (ESP_RES_ERROR);
        }
    }

    return (ESP_RES_TIMEOUT); 
}

uint8_t ESP8266_SearchAP(APInfo_t *apInfo, uint8_t *listNbr, uint8_t maxNbr)
{
    return (ESP_RES_TIMEOUT);
}

uint8_t ESP8266_ClearAPList(APInfo_t *apInfo, uint8_t listNbr)
{
    return (ESP_RES_TIMEOUT);
}

/* pBuff[64] */
uint8_t ESP8266_GetSSID(uint8_t *ssid, uint32_t pSize)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_CCWJAP);
    ret = esp_WaitResponse(ssid, pSize, "OK|ERROR", 500);
    if(ret == 1) {//ok
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);     
}

/* 0:normal, 1:pass through */
uint8_t ESP8266_SetXsferMode(uint8_t mode)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_SCIPMODE, mode);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//ok
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);   
}

uint8_t ESP8266_GetConnectedIP(IPConnect_t *ip)
{
    return (ESP_RES_TIMEOUT);
}

uint8_t ESP8266_GetConnectState(uint8_t *state)
{    
    return (ESP_RES_TIMEOUT); 
}

/* 0:single, 1:multiple connect */
uint8_t ESP8266_SetConnectMode(uint8_t mode)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_SCIPMUX, mode);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//OK
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT);     
}

uint8_t ESP8266_SetServerIP(const char *ip)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_SCIPAP, ip);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//OK
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT); 	
}

uint8_t ESP8266_SetStationIP(const char *ip)
{
    uint8_t ret = 0;

    esp_SendATCmd(CMD_SCIPSTA, ip);
    ret = esp_WaitResponse(NULL, 0, "OK|ERROR", 500);
    if(ret == 1) {//OK
        return (ESP_RES_OK);
    } else if(ret == 2) {//error
        return (ESP_RES_ERROR);
    }

    return (ESP_RES_TIMEOUT); 	
}


/* single connect 
*   @param type : "TCP"/"UDP"
*   @param ip   : "192.168.1.102"
*   @param port : 
*/
uint8_t ESP8266_IPConnect0(const char *type, const char *ip, uint16_t port)
{
    uint8_t ret = 0, waitTimes = 5;

    esp_SendATCmd(CMD_SCIPSTART0, type, ip, port);
    /* wait OK */
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, "OK|CONNECTED|ERROR", 8000);
        if((ret == 1) || (ret == 2)) { //ok
            return (ESP_RES_OK);
        } else if(ret == 3) {//error
            return (ESP_RES_ERROR);
        }
    }

    return (ESP_RES_TIMEOUT);     
}

/* multiple connect
*   @param id   : 0~4
*   @param type : "TCP"/"UDP"
*   @param ip   : "192.168.2.2"
*   @param port : 0~65535
*/
uint8_t ESP8266_IPConnect1(uint8_t id, const char *type, const char *ip, uint16_t port)
{
    uint8_t ret = 0, waitTimes = 5;

    esp_SendATCmd(CMD_SCIPSTART1, id, type, ip, port);
    /* wait OK */
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, "OK|CONNECTED|ERROR", 8000);
        if((ret == 1) || (ret == 2)) { //ok
            return (ESP_RES_OK);
        } else if(ret == 3) {//error
            return (ESP_RES_ERROR);
        }
    }

    return (ESP_RES_TIMEOUT);     
}

/* CMD_SCIPSEND0 */
uint8_t ESP8266_IPSend0(uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint8_t ret = 0, waitTimes = 5;

    /* over size? or data? */
    if((size > ESP_PACKET_SIZE) || (!pData)) {
        return (ESP_RES_ERROR);
    }
    /* send cmd */
    esp_SendATCmd(CMD_SCIPSEND0, size);
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, ">|ERROR", 2000);
        if(ret == 1) {//ok
            break;
        } else if(ret == 2) {//error
            return (ESP_RES_ERROR);
        }
    }
    /* AT Cmd timeout, mark as error */
    if(ret == 0) {//error
    	printf("ESP8266_IPSend0 ret = %d\r\n", ret);
        return (ESP_RES_ERROR);
    }
    /* send data */
    esp_SendData(pData, size);
    /* wait send compeleted */
    if(ESP_WaitCondition(timeout, esp_GetTxCplt) == TRUE) 
    {
    	while(waitTimes--)
	    {
	        ret = esp_WaitResponse(NULL, 0, "SEND OK", 5000);
	        if(ret == 1) {//ok
	            return (ESP_RES_OK);
	        }
	    }
    }

    return (ESP_RES_TIMEOUT);    
}

/* CMD_SCIPSEND0 */
uint8_t ESP8266_IPSend1(uint8_t id, uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint8_t ret = 0, waitTimes = 5;

    /* over size? on data? */
    if((size > ESP_PACKET_SIZE) || (!pData)) {
        return (ESP_RES_ERROR);
    }
    /* send cmd */
    esp_SendATCmd(CMD_SCIPSEND1, id, size);
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, ">|ERROR", 2000);
        if(ret == 1) {//ok
            break;
        } else if(ret == 2) {//error
            return (ESP_RES_ERROR);
        }
    }
    /* AT Cmd timeout, mark as error */
    if(ret == 0) {//error
        return (ESP_RES_ERROR);
    }
    /* send data */
    esp_SendData(pData, size);
    /* wait send compeleted */
    if(ESP_WaitCondition(timeout, esp_GetTxCplt) == TRUE) 
    {
    	while(waitTimes--)
	    {
	        ret = esp_WaitResponse(NULL, 0, "SEND OK", 5000);
	        if(ret == 1) {//ok
	            return (ESP_RES_OK);
	        }
	    }
    }
    
    return (ESP_RES_TIMEOUT);    
}

uint8_t ESP8266_StartPassThrough(void)
{
    uint8_t ret = 0, waitTimes = 5;

    esp_SendATCmd(CMD_PCIPSEND);
    while(waitTimes--)
    {
        ret = esp_WaitResponse(NULL, 0, ">|ERROR", 2000);
        if(ret == 1) {//ok
            return (ESP_RES_OK);
        } else if(ret == 2) {//error
            return (ESP_RES_ERROR);
        }
    }    

    return (ESP_RES_TIMEOUT);
}

/* one packet, 2048KBytes max */
uint8_t ESP8266_PassThrough0(uint8_t *pData, uint16_t size, uint32_t timeout)
{
    /* over size? on data? */
    if((size > ESP_PACKET_SIZE) || (!pData)) {
        return (ESP_RES_ERROR);
    }
    /* send cmd */
    esp_SendData(pData, size);
    /* wait send compeleted */
    if(ESP_WaitCondition(timeout, esp_GetTxCplt) == FALSE) {
		return (ESP_RES_TIMEOUT);
    }
    
    return (ESP_RES_OK);  
}

/* multiple packets, 2048KBytes/packet */
uint8_t ESP8266_PassThrough1(uint8_t *pData, uint16_t size, uint32_t timeout)
{	
    uint8_t *tmpPtr = pData;
    uint32_t sendSize = 0, relSize = size;

    /* no data? */
    if((size == 0) || (!pData)) {
        return (ESP_RES_ERROR);
    }
    /* send */
    while(relSize)
    {
        if(relSize / ESP_PACKET_SIZE) {//>2048
            sendSize = ESP_PACKET_SIZE;
        } else {
            sendSize = relSize;
        }
        /* send data */
        esp_SendData(tmpPtr, sendSize);
        /* wait send compeleted */
	    if(ESP_WaitCondition(timeout, esp_GetTxCplt) == FALSE) {
		 	return (ESP_RES_TIMEOUT);
	    }
        /* delay */
        TimDelayMs(22);
        /* next packet */
        relSize -= sendSize;
        tmpPtr += sendSize;
    }
    
    return (ESP_RES_OK);  
}

uint8_t ESP8266_StopPassThrough(void)
{
    esp_SendATCmd(CMD_PCIPSENDEND);
    TimDelayMs(5);

    return (ESP_RES_OK);
}

/* Put message to pool */
bool ESP8266_PutMessageSendPool(uint8_t *data, uint32_t size)
{
    return esp_MessagePoolPut(EspSendMesgPool, data, size);
}

/* Get message from pool */
uint32_t ESP8266_GetMessageSend(uint8_t *buffer)
{
    return esp_MessagePoolGet(EspSendMesgPool, buffer);
}

uint32_t ESP8266_GetMessageRecv(uint8_t *buffer)
{
    return esp_MessagePoolGet(EspRecvMesgPool, buffer);
}

bool IsEsp8266InXsfer(void)
{
    return ((IsEspRxCplt == ESP_XSFER_WAITCPLT)
            || (IsEspTxCplt == ESP_XSFER_WAITCPLT));
}

/********* PRIVATE FUNCTION ***************/
static void esp_IoInit(void)
{
	GPIO_InitTypeDef GPIO_Init;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	GPIO_Init.Pin = ESP_REST_PIN;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_MEDIUM;

	HAL_GPIO_Init(ESP_REST_PORT, &GPIO_Init);
}

static bool esp_GetRxCplt(void)
{
    return (IsEspRxCplt == ESP_XSFER_CPLT);
}

static bool esp_GetTxCplt(void)
{
    return (IsEspTxCplt == ESP_XSFER_CPLT);
}

static void esp_SendData(uint8_t *pData, uint16_t size)
{
    IsEspTxCplt = ESP_XSFER_WAITCPLT;
    ATKUart_TransmitIT(pData, size);
}

static bool esp_SendATCmd(const char *atcmd, ...)
{
    uint16_t size;
    va_list args;

    va_start(args, atcmd);
    vsnprintf((char*)EspCmdBuffer, ESP_CMDBUFF_SIZE, atcmd, args);
    va_end(args);
    /* check data size */
    size = myStrlen((const char *)EspCmdBuffer);
    printf("ESP8266 send cmd [%d]: %s\r\n", size, EspCmdBuffer);
    /* send cmd packet */
    //if(ESP_WaitCondition(10000, esp_GetTxCplt) == TRUE) {
        esp_SendData((uint8_t*)EspCmdBuffer, size);
    //} else {
    //    return (FALSE);
    //}

	return (TRUE);
}

/*
*   @param response : "resp1|resp2|resp3"
*   @param timeout : in ms
*   @return : 0-timeout/error, 1-resp1, 2-resp2, 3-resp3
*/
static uint8_t esp_WaitResponse(uint8_t *pBuff, uint16_t pSize, const char *response, uint32_t timeout)
{
    uint32_t tmpSize = 0;
    char *tmpPtr0 = NULL, *tmpPtr1 = NULL;
    char resp1[32], resp2[32], resp3[32];
    uint8_t ret = 0;

    /* clear rx buffer, for wait response */
    myMemset(EspDataBuffer, 0, ESP_DATABUFF_SIZE);
	IsEspRxCplt = ESP_XSFER_WAITCPLT;
	EspRxDataSize = 0;
	EspRxByteIdx = 0;
    
    if(ESP_WaitCondition(timeout, esp_GetRxCplt) == TRUE)
    {
        IsBufferBusy = TRUE;
        /* check */
        if(!pBuff && !response) {
            ret = 0;
            goto WaitResponseEnd;
        }

        /* check buffer */
        if(EspDataBuffer[0] == '\0') {
            ret = 0;
            goto WaitResponseEnd;
        }

        /* skip "\r\n" */
        if((EspDataBuffer[0] == '\r') && (EspDataBuffer[1] == '\n')) {
            tmpPtr0 = (char*)EspDataBuffer+2;
        } else {
            tmpPtr0 = (char*)EspDataBuffer;
        }

        /* get return param */
        if(pBuff)
        {
            tmpPtr1 = myStrstr(tmpPtr0, "OK\r\n");
            if((tmpPtr1) && (tmpPtr1 != tmpPtr0)) 
            {
                tmpSize = (uint32_t)tmpPtr1 - (uint32_t)tmpPtr0;
                tmpSize = (tmpSize < pSize) ? tmpSize : pSize;
                myMemcpy(pBuff, (const uint8_t*)tmpPtr0, tmpSize);
                pBuff[tmpSize] = '\0';
            }
            else
            {
                //no param
            }
        }
        
        /* check response */
        if(response)
        {
            uint8_t i = 0;
            
            /* get resp0 */
            if(response[0] != '\0')
            {
                for(i=0; (response[0]!='|') && (response[0]!='\0'); i++) {
                    resp1[i] = *response++;
                }
                resp1[i+1] = '\0';
                //printf("resp1 = %s\r\n", resp1);
            }
            /* get resp1 */
            if(response[0] == '|')
            {
                response++;//skip '|'
                for(i=0; (response[0]!='|') && (response[0]!='\0'); i++) {
                    resp2[i] = *response++;
                }
                resp2[i+1] = '\0';
                //printf("resp2 = %s\r\n", resp2);
            }
            /* get resp2 */
            if(response[0] == '|')
            {
                response++;//skip '|'
                for(i=0; (response[0]!='|') && (response[0]!='\0'); i++) {
                    resp3[i] = *response++;
                }
                resp3[i+1] = '\0';
                //printf("resp3 = %s\r\n", resp3);
            }
            /* check response */
            tmpPtr1 = tmpPtr0+tmpSize;
            //printf("recv response = %s\r\n", tmpPtr1);
            if((resp1[0] != '\0') && (myStrstr(tmpPtr1, resp1) != NULL)) {
                ret = 1;
            }
            else if((resp2[0] != '\0') && (myStrstr(tmpPtr1, resp2) != NULL)) {
                ret = 2;
            }
            else if((resp3[0] != '\0') && (myStrstr(tmpPtr1, resp3) != NULL)) {
                ret = 3;
            } else {
                ret = 0;
            }
        }
    } 
/* process end */    
WaitResponseEnd:
    /* clear rx status */
    IsBufferBusy = FALSE; 
    IsEspRxCplt = ESP_XSFER_FREE;
    EspRxDataSize = 0;
    EspRxByteIdx = 0;
    
    return (ret);
}

static void esp_RxParser(uint8_t data)
{
    if(IsBufferBusy == FALSE)
    {
        EspDataBuffer[EspRxByteIdx] = data;
        //IsEspRxCplt = ESP_XSFER_WAITCPLT;
        /* over size */
        if(++EspRxByteIdx == ESP_DATABUFF_SIZE)
        {
            /* mark as oversize */
            IsEspRxCplt = ESP_XSFER_OVERSIZE;
        }
    }
}

static void esp_RxCpltProc(void)
{
	uint8_t tmpStr[16] = {0}, *tmpPtr = NULL;
	uint32_t DataSize = 0, i = 0;

    if(IsEspRxCplt == ESP_XSFER_OVERSIZE) 
    {
        /* clear this received data */
        printf("ESP8266 receive oversize\r\n");
        myMemset(EspDataBuffer, 0, ESP_DATABUFF_SIZE);  
        IsEspRxCplt = ESP_XSFER_FREE;
    	EspRxDataSize = 0;
      	EspRxByteIdx = 0;
    }
	else if(IsEspRxCplt == ESP_XSFER_WAITCPLT)
	{
	    /* wait response after AT Cmd */
		printf("ESP8266 receive [%d]: %s\r\n", EspRxByteIdx, EspDataBuffer);
	    IsEspRxCplt = ESP_XSFER_CPLT;
	    EspRxDataSize = EspRxByteIdx;
	    EspRxByteIdx = 0;
    }
    else if(IsEspRxCplt == ESP_XSFER_FREE)
    {
    	/* TCP/UDP and other received message */
        /*  TCP Data
        *   Format : +IPD,[size]:[data]
        */
        printf("ESP8266 receive [%d]: %s\r\n", EspRxByteIdx, EspDataBuffer);
        //skip "\r\n"
        tmpPtr = EspDataBuffer;
        if((tmpPtr[0]=='\r') && ((tmpPtr[1]=='\n'))) {
            tmpPtr += 2;
        }
        //check if recv data is TCP data
        if((tmpPtr[0]=='+') && (tmpPtr[1]=='I') 
            && (tmpPtr[2]=='P') && (tmpPtr[3]=='D') && (tmpPtr[4]==','))
        {
            tmpPtr += strlen("+IPD,");
            //get data size
            while((*tmpPtr != ':') && (i < 5)) {//max 2048
				tmpStr[i++] = *tmpPtr++;
			}

			if(i != 5)
			{
    			tmpStr[i] = '\0';
    			DataSize = atoi((char*)tmpStr);
    			tmpPtr++;//skip ':'
    			//printf("TCP DataSize[%s-%d] = %s\r\n", tmpStr, DataSize, tmpPtr);
    			if(DataSize)
    			{
    				if(esp_MessagePoolPut(EspRecvMesgPool, tmpPtr, DataSize) == 0) 
    				{
    					//printf("esp_MessagePoolPut() error\r\n");
    				}
    			}
			}
        }
    	/* clear RX Buffer */
		myMemset(EspDataBuffer, 0, ESP_DATABUFF_SIZE); 
		EspRxDataSize = 0;
		EspRxByteIdx = 0;
    }
}

static void esp_TxCpltProc(void)
{
    IsEspTxCplt = ESP_XSFER_CPLT;
}

static void esp_ErrorProc(void)
{
    IsEspTxCplt = ESP_XSFER_ERROR;
}

static bool esp_MessagePoolPut(EspMesg_t *pool, uint8_t *data, uint32_t size)
{
    uint8_t idx = 0;
    uint8_t *dataStore = NULL;

	if(!pool || !data || (size>ESP_MAX_MESGSIZE)) {
		return (FALSE);
	}

    for(idx=0; idx<ESP_MAX_MESG; idx++)
    {
    	if(pool[idx].data == NULL)
    	{
    		dataStore = (uint8_t*)myMalloc(MLCSRC_AXISRAM, size);
    		if(dataStore)
    		{
    			myMemset(dataStore, 0, size);
    			myMemcpy(dataStore, data, size);
    			//printf("dataStore = %s\r\n", dataStore);
    			pool[idx].data = dataStore;
    			pool[idx].size = size;
				return (TRUE);
    		}
    	}
    }

    return (FALSE);
}

/* the 'buffer' must big enough */
static uint32_t esp_MessagePoolGet(EspMesg_t *pool, uint8_t *buffer)
{
    int idx = 0;
    uint32_t dataSize = 0;

    if(!pool || !buffer) {//param error
    	return (0);
   	}

    for(idx=ESP_MAX_MESG-1; idx>=0; idx--)
    {
    	//printf("%d,", idx);
    	if(pool[idx].data != NULL)
    	{	
    		//printf("dataSize = %d\r\n", pool[idx].size);
    		dataSize = pool[idx].size;
    		myMemcpy(buffer, (const uint8_t*)pool[idx].data, dataSize);
			myFree(MLCSRC_AXISRAM, pool[idx].data);
    		//pool[idx].data = NULL;
    		pool[idx].size = 0;

    		return (dataSize);
    	}
    }

    return (0);
}


