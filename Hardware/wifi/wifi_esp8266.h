
#ifndef _WIFI_ESP8266_H
#define _WIFI_ESP8266_H

#include "public.h"

/* ESP8266 Command [test cmd, check cmd, setup cmd, perform cmd] 
*/
/* module test */
#define CMD_TEST                    "AT\r\n"//return OK
/*  */
#define CMD_ATE                     "ATE%d\r\n"
/* module reset */
#define CMD_RESET                   "AT+RST\r\n"//return ready
/* module UART config */
#define CMD_PUART                   "AT+UART=%d,%d,%d,%d,%d\r\n"//<rate><bits><stopbits><parity><flow>
/* wifi mode */
#define CMD_TCWMODE                 "AT+CWMODE=?\r\n"//get supported mode
#define CMD_CCWMODE                 "AT+CWMODE?\r\n"//get current mode
#define CMD_SCWMODE                 "AT+CWMODE=%d\r\n"//set mode, <mode>
/* join AP */
#define CMD_CCWJAP                  "AT+CWJAP?"//get current AP-SSID
#define CMD_SCWJAP                  "AT+CWJAP=\"%s\",\"%s\"\r\n"//join AP-SSID, <ssid><pwd>
/* AP list */
#define CMD_SCWLAP                  "AT+CWLAP=\"%s\",\"%s\",%d\r\n"//check AP valid? <ssid><mac><ch>
#define CMD_PCWLAP                  "AT+CWLAP\r\n"//AP list
/* quit AP */
#define CMD_TCWQAP                  "AT+CWQAP=?\r\n"//AP Connect?
#define CMD_PCWQAP                  "AT+CWQAP\r\n"//AP Disconnect
/* AP setup */
#define CMD_CCWSAP                  "AT+CWSAP\r\n"//get current AP setup
#define CMD_SCWSAP                  "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n"//AP setup, <ssid><pwd><ch><ecn>
/* get connected IP */
#define CMD_PCWLIF                  "AT+CWLIF\r\n"//get connected IP
/* get module IP */
#define CMD_PCIFSR                  "AT+CIFSR\r\n"//
/* set IP mode */
#define CMD_SCIPMODE                "AT+CIPMODE=%d\r\n"//
/* get IP connect status */
#define CMD_TCIPSTATUS              "AT+CIPSTATUS=?\r\n"//return OK
#define CMD_PCIPSTATUS              "AT+CIPSTATUS\r\n"//get connect status
/* set IP */
#define CMD_SCIPAP					"AT+CIPAP=\"%s\"\r\n"
#define CMD_SCIPSTA					"AT+CIPSTA=\"%s\"\r\n"
/* TCP/UDP connect */
#define CMD_SCIPMUX                 "AT+CIPMUX=%d\r\n"//enable/disable multiple connect
#define CMD_TCIPSTART               "AT+CIPSTART=?\r\n"//get support connect settings
#define CMD_SCIPSTART0              "AT+CIPSTART=\"%s\",\"%s\",%d\r\n"//single connect, <type><ip><port>
#define CMD_SCIPSTART1              "AT+CIPSTART=%d,\"%s\",\"%s\",%d\r\n"//multiple connect, <id><type><ip><port> 
/* data transmit */
#define CMD_TCIPSEND                "AT+CIPSEND=?\r\n"//return OK
#define CMD_SCIPSEND0               "AT+CIPSEND=%d\r\n"//set data length, single connect, <length>
#define CMD_SCIPSEND1               "AT+CIPSEND=%d,%d\r\n"//set data length, multiple connect, <id><length>
#define CMD_PCIPSEND                "AT+CIPSEND\r\n"/* after receive "\r>", send data.
                                                     * 20ms each packet, 2048kbyte/packet */
#define CMD_PCIPSENDEND             "+++"
/* TCP/UDP disconnect */                                                     
#define CMD_TCIPCLOSE               "AT+CIPCLOSE=?\r\n"//return OK
#define CMD_PCIPCLOSE0              "AT+CIPCLOSE\r\n"//use on single connect mode
#define CMD_PCIPCLOSE1              "AT+CIPCLOSE=%d\r\n"//use on multiple connect mode, <id>
/* server setup */
#define CMD_SCIPSERVER              "AT+CIPSERVER=%d,%d\r\n"//open/close IP server, <mode><port>, need reboot

//#define ESP8266_OKCRLF              "OK\r\n"

#define ESP_XSFER_CPLT              (0)
#define ESP_XSFER_WAITCPLT          (1)//wait data xsfer complete
#define ESP_XSFER_FREE              (2)
#define ESP_XSFER_TIMEOUT           (3)
#define ESP_XSFER_OVERSIZE          (4)
#define ESP_XSFER_ERROR             (5)
#define ESP_XSFER_UNKNOW            (6)


/*  mode
*/
enum
{
    WLANMODE_STATION = 1,
    WLANMODE_AP,
    WLANMODE_AP_STATION,
};

/*  AP ecn
*/
enum
{
    AP_ECN_OPEN = 0,
    AP_ECN_WEP,
    AP_ECN_WPA_PSK,
    AP_ECN_WPA2_PSK,
    AP_ECN_WPA_WPA2_PSK,
};

/*  AP connect status
*/
enum
{
    APSTATUS_DISCONNECT = 0,
    APSTATUS_CONNECTED,
};

/*  connect mode
*/
enum
{
    IPCONNECT_MODE_SINGLE = 0,
    IPCONNECT_MODE_MULTIPLE,
};

/*  TCP/UDP
*/
#define CONNECTED_TCP   "TCP"
#define CONNECTED_UDP   "UDP"
enum
{
    IPCONNECT_TYPE_UDP=  0,
    IPCONNECT_TYPE_TCP,  
};

/*  connect status
*/
enum
{
    IPSTATUS_GETIP = 2,
    IPSTATUS_CONNECTED,
    IPSTATUS_DISCONNECTED,
};

/*  module mode
*/
enum
{
    IPMODE_CLIENT = 0,
    IPMODE_SERVER,
};

/*  server status
*/
enum
{
    IPSERVER_CLOSE = 0,
    IPSERVER_OPEN,
};

/*  wait response
*/
enum
{
    ESP_RES_OK = 0,
    ESP_RES_ERROR,
    ESP_RES_TIMEOUT,
    ESP_RES_BUSY,
};

/*  IP connect 
*/
typedef struct
{
    uint8_t  ip[4];
    uint16_t port;
    uint8_t  id;
    uint8_t  type;
}IPConnect_t;

/*  IP server
*/
typedef struct
{
    uint8_t  ip[4];
    uint16_t port;
    uint8_t  id;
    uint8_t  state;
}IPServer_t;

/*  AP info list
*/
typedef struct
{
    uint8_t ssid[64];
    uint8_t mac[32];
    uint8_t ecn;
    uint8_t rssi;
    uint8_t channel;
}APInfo_t;

/*  TCP/UDP Message
*/
#define ESP_MAX_MESG        (30)
#define ESP_MAX_MESGSIZE    (2048U)

typedef struct
{
	uint8_t *data;
	uint32_t size;
}EspMesg_t;

/**** PUBLIC FUNCTION ****/
uint8_t ESP8266_Init(void);
void ESP8266_DeInit(void);
uint8_t ESP8266_UartConfig(uint8_t baudRate, uint8_t bits, uint8_t stopbits, uint8_t pority, uint8_t flowc);
uint8_t ESP8266_TestATCmd(void);
uint8_t ESP8266_EchoOnoff(uint8_t on);
uint8_t ESP8266_Reset(void);
uint8_t ESP8266_SetWlanMode(uint8_t mode);
uint8_t ESP8266_JoinAP(uint8_t *ssid, uint8_t *pwd);
uint8_t ESP8266_GetSSID(uint8_t *ssid, uint32_t pSize);
uint8_t ESP8266_QuitAP(void);

uint8_t ESP8266_SetXsferMode(uint8_t mode);
uint8_t ESP8266_GetConnectedIP(IPConnect_t *ip);
uint8_t ESP8266_GetConnectState(uint8_t *state);
uint8_t ESP8266_SetConnectMode(uint8_t mode);
uint8_t ESP8266_SetServerIP(const char *ip);
uint8_t ESP8266_SetStationIP(const char *ip);
uint8_t ESP8266_IPConnect0(const char *type, const char  *ip, uint16_t port);
uint8_t ESP8266_IPConnect1(uint8_t id, const char *type, const char *ip, uint16_t port);
uint8_t ESP8266_IPSend0(uint8_t *pData, uint16_t size, uint32_t timeout);
uint8_t ESP8266_IPSend1(uint8_t id, uint8_t *pData, uint16_t size, uint32_t timeout);
uint8_t ESP8266_StartPassThrough(void);
uint8_t ESP8266_PassThrough0(uint8_t *pData, uint16_t size, uint32_t timeout);
uint8_t ESP8266_PassThrough1(uint8_t *pData, uint16_t size, uint32_t timeout);
uint8_t ESP8266_StopPassThrough(void);

bool ESP8266_PutMessageSendPool(uint8_t *data, uint32_t size);
uint32_t ESP8266_GetMessageSend(uint8_t *buffer);
uint32_t ESP8266_GetMessageRecv(uint8_t *buffer);
bool IsEsp8266InXsfer(void);
#endif

