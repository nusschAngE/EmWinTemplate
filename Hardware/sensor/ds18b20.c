
#include "stm32h7xx.h"

#include "ds18b20.h"
#include "delay.h"

/**
*   one DS18B20 only
*/

/* DQ */
#define TDQ_GPIO    GPIOB
#define TDQ_PIN     GPIO_PIN_12


/* CMD */
#define DSCMD_SEARCH_ROM        (0xF0)
#define DSCMD_READ_ROM          (0x33)
#define DSCMD_MATCH_ROM         (0x55)
#define DSCMD_SKIP_ROM          (0xCC)
#define DSCMD_SEARCH_ALARM      (0xEC)

#define DSCMD_CONVERT           (0x44)
#define DSCMD_WRITE_SPAD        (0x4E)
#define DSCMD_READ_SPAD         (0xBE)
#define DSCMD_COPY_SPAD         (0x48)
#define DSCMD_RECALL_EEPROM     (0xB8)
#define DSCMD_READ_PWRMODE      (0xB4)

/* config register */
#define DS_CONFIG_REG           (0x1f)

/* in ms */
#define DS_CONVERT_TIME_9BIT    (95)//93.75ms
#define DS_CONVERT_TIME_10BIT   (190)//187.5ms
#define DS_CONVERT_TIME_11BIT   (380)//375ms
#define DS_CONVERT_TIME_12BIT   (760)//750ms

/*  STATIC
*/
static inline void TDQ_ConfigAsOutput(void);
static inline void TDQ_ConfigAsInput(void);
static inline uint8_t TDQ_Read(void);
static inline void TDQ_Set(uint8_t sta);

static uint8_t ds_reset(void);
static void ds_sendbyte(uint8_t sVal);
static uint8_t ds_readbyte(void);


#define DS18B20_DELAY(x)        TimDelayUs(x)

/***************** PUBLIC FUNCTION ********************/
uint8_t DS18B20_Init(uint8_t th, uint8_t tl, uint8_t precise)
{
    uint8_t tmp = 0;
    
    if(ds_reset() == 0)
    {
        ds_sendbyte(DSCMD_SKIP_ROM);
        ds_sendbyte(DSCMD_WRITE_SPAD);
        ds_sendbyte(th);
        ds_sendbyte(tl);
        /* 0x0**11111 */
        tmp = DS_CONFIG_REG | (precise << 5);
        ds_sendbyte(tmp);

        return (1);
    }

    return (0);
}

uint8_t DS18B20_StartConvert(void)
{
    if(ds_reset() == 0)
    {
        ds_sendbyte(DSCMD_SKIP_ROM);
        ds_sendbyte(DSCMD_CONVERT);
        ds_reset();
        
        return (1);
    }

    return (0);
}

/* 0 : on convertion, 1 : convert done */
uint8_t DS18B20_CheckConvert(void)
{
    return (TDQ_Read());
}

uint8_t DS18B20_ReadTemp(uint16_t *tempVal)
{
    uint8_t msb, lsb;

    if(ds_reset() == 0)
    {
        ds_sendbyte(DSCMD_SKIP_ROM);
        ds_sendbyte(DSCMD_READ_SPAD);
        lsb = ds_readbyte();//lsb first
        msb = ds_readbyte();

        *tempVal = ((uint16_t)msb << 8) | lsb;

        return (1);
    }
    
    return (0);
}

float DS18B20_GetTempFloat(uint16_t tempVal)
{
    uint8_t pm = 0;
    float tmp = 0.0;

    pm = (tempVal & 0x8000) ? 1 : 0;//1 : minus, 0 : plus
    tmp = (tempVal & 0x7fff) * 0.625;

    return (pm) ? -tmp : tmp;
}

int DS18B20_GetTempInt(uint16_t tempVal)
{
    uint8_t pm = 0;
    int tmp = 0;

    pm = (tempVal & 0x8000) ? 1 : 0;//1 : minus, 0 : plus
    tmp = (int)((tempVal & 0x7fff) * 0.625);

    return (pm) ? -tmp : tmp;
}


/***************** PRIVATE FUNCTION *******************/
static inline void TDQ_ConfigAsOutput(void)
{
    GPIO_InitTypeDef GPIO_Init;

    GPIO_Init.Pin = TDQ_PIN;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(TDQ_GPIO, &GPIO_Init);
}

static inline void TDQ_ConfigAsInput(void)
{
    GPIO_InitTypeDef GPIO_Init;

    GPIO_Init.Pin = TDQ_PIN;
    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(TDQ_GPIO, &GPIO_Init);
}

static inline uint8_t TDQ_Read(void)
{
    return HAL_GPIO_ReadPin(TDQ_GPIO, TDQ_PIN) ? 1 : 0;
}

static inline void TDQ_Set(uint8_t sta)
{
    HAL_GPIO_WritePin(TDQ_GPIO, TDQ_PIN, sta ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline uint8_t ds_readbit(void)
{
    uint8_t bit = 0;

    TDQ_ConfigAsOutput();
    DS18B20_DELAY(5);
    TDQ_Set(0);
    DS18B20_DELAY(10);
    TDQ_Set(1);
    TDQ_ConfigAsInput();
    DS18B20_DELAY(10);
    bit = TDQ_Read();
    DS18B20_DELAY(45);
    return bit;
}

static inline void ds_sendbit(uint8_t bit)
{
    if(bit)
    {
        TDQ_Set(0);
        DS18B20_DELAY(5);
        TDQ_Set(1);
        DS18B20_DELAY(60);
    }
    else
    {
        TDQ_Set(0);
        DS18B20_DELAY(60);
        TDQ_Set(1);
        DS18B20_DELAY(5);
    }
}

static uint8_t ds_reset(void)
{
    uint16_t wait = 0xffff;

    //TDQ_Set(1);
    TDQ_ConfigAsOutput();
    DS18B20_DELAY(5);
    /* reset */
    TDQ_Set(0);
    DS18B20_DELAY(500);
    /* check ACK */
    //TDQ_Set(1);
    TDQ_ConfigAsInput();
    DS18B20_DELAY(20);
    while(TDQ_Read())
    {
        DS18B20_DELAY(5);
        if(--wait == 0) {return (1);}//nack
    }
    DS18B20_DELAY(60);

    return (0);//ack
}

static void ds_sendbyte(uint8_t sVal)
{
    uint8_t i;

	//TDQ_Set(1);
    TDQ_ConfigAsOutput();   
    DS18B20_DELAY(10);
    for(i = 0; i < 8; i++)
    {
    	TDQ_Set(1);
        DS18B20_DELAY(5);
        TDQ_Set(0);
        DS18B20_DELAY(13);
        TDQ_Set(sVal & 0x01);
        sVal >>= 1;
        DS18B20_DELAY(50);//must > 65us
    }
}

static uint8_t ds_readbyte(void)
{
    uint8_t rVal = 0, i;
        
    for(i = 0; i < 8; i++)
    {
        //TDQ_Set(1);
        TDQ_ConfigAsOutput();
        DS18B20_DELAY(5);
        TDQ_Set(0);
        DS18B20_DELAY(13);
        TDQ_ConfigAsInput();
        DS18B20_DELAY(25);
		/* MSB First */ 
        rVal >>= 1;
        if(TDQ_Read()) {rVal |= 0x80;}
        DS18B20_DELAY(40);//must > 60us
    }

    return rVal;
}

#if 0
static uint8_t ds_CaculateCRC(uint16_t temp)
{
    
}

static uint8_t ds_ReadCRC(uint8_t *crc)
{

}
#endif

