
#include "stm32h7xx.h"
#include "sys_i2c.h"
#include "delay.h"

#define IIC_DELAY(x)    TimDelayUs(x)
#define IOCTRL_DELAY    (5)
#define FREQCTRL_DELAY  (3)//US 

#define SCL_GPIO        GPIOH
#define SCL_PIN         GPIO_PIN_4

#define SDA_GPIO        GPIOH
#define SDA_PIN         GPIO_PIN_5

/*  */
//#define IIC_SCL_H()         HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET)
//#define IIC_SCL_L()         HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET)

//#define IIC_SDA_H()         HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET)
//#define IIC_SDA_L()         HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET)

//#define IIC_SDA_READ()      HAL_GPIO_ReadPin(SDA_GPIO, SDA_PIN)


/*  STATIC 
*/
static inline void IIC_SDA_ConfigAsOutput(void);
static inline void IIC_SDA_ConfigAsInput(void);
static inline uint8_t IIC_SDA_Read(void);
static inline void IIC_SDA_Set(uint8_t sta);
static inline void IIC_SCL_Set(uint8_t sta);


//static bool SystemIIC_Init = FALSE;


/***************** PUBLIC FUNCTION**********************/
void SYSI2C_Init(void)
{
    GPIO_InitTypeDef GPIO_Init;

    //if(SystemIIC_Init != TRUE)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();

        GPIO_Init.Pin = SCL_PIN | SDA_PIN;
        GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Init.Pull = GPIO_PULLUP;
        GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOH, &GPIO_Init);

        //SystemIIC_Init = TRUE;
    }
}

void SYSI2C_Start(void)
{
    IIC_SDA_Set(1);
    IIC_SCL_Set(1);
    IIC_SDA_ConfigAsOutput();
    IIC_DELAY(IOCTRL_DELAY);
    IIC_SDA_Set(0);
    IIC_DELAY(FREQCTRL_DELAY);
    IIC_SCL_Set(0);
}

void SYSI2C_SendByte(uint8_t sVal)
{
    uint8_t t;   
    
    IIC_SCL_Set(0);
	IIC_SDA_ConfigAsOutput(); 	    
	IIC_DELAY(IOCTRL_DELAY);
	for(t = 0; t < 8; t++)
    {
		IIC_SDA_Set(sVal & 0x80);//MSB first
		sVal <<= 1; 		
		
		IIC_SCL_Set(1); 
		IIC_DELAY(FREQCTRL_DELAY);
		IIC_SCL_Set(0);	
		IIC_DELAY(FREQCTRL_DELAY);
    }
}

uint8_t SYSI2C_WaitACK(void)
{
    uint8_t cnt = 0;
	
    IIC_SDA_Set(1);
    IIC_SDA_ConfigAsInput();    
    IIC_SCL_Set(1);
    IIC_DELAY(IOCTRL_DELAY);
    while(IIC_SDA_Read())
    {
        if(++cnt > 250)
        {
            SYSI2C_Stop();
            return (1);
        }
        IIC_DELAY(FREQCTRL_DELAY);
    }

    return (0);    
}

uint8_t SYSI2C_ReadByte(void)
{
	uint8_t i, receive = 0;

	IIC_SCL_Set(0); 
 	IIC_SDA_ConfigAsInput();
	IIC_DELAY(IOCTRL_DELAY);
	for(i = 0; i < 8; i++ )
	{ 
		IIC_SCL_Set(0); 	    	   
		IIC_DELAY(FREQCTRL_DELAY);
		IIC_SCL_Set(1);	 
		IIC_DELAY(FREQCTRL_DELAY);
		receive <<= 1;
		if(IIC_SDA_Read())
		{
		    receive |= 0x01;   
		}
	}	
	    
 	return (receive);    
}

void SYSI2C_SendACK(void)
{
	IIC_SCL_Set(0);
	IIC_DELAY(FREQCTRL_DELAY);
	IIC_SDA_Set(0);
	IIC_SDA_ConfigAsOutput();	
	IIC_DELAY(IOCTRL_DELAY);
	IIC_SCL_Set(1);
	IIC_DELAY(FREQCTRL_DELAY);
}

void SYSI2C_SendNACK(void)
{
	IIC_SCL_Set(0);
	IIC_DELAY(FREQCTRL_DELAY);
	IIC_SDA_Set(1);
	IIC_SDA_ConfigAsOutput();	
	IIC_DELAY(IOCTRL_DELAY);
	IIC_SCL_Set(1);
	IIC_DELAY(FREQCTRL_DELAY);
}

void SYSI2C_Stop(void)
{
    IIC_SCL_Set(0);
    //IIC_DELAY(2);
    IIC_SDA_Set(0);    
    IIC_SDA_ConfigAsOutput();
    IIC_DELAY(IOCTRL_DELAY);

    IIC_SCL_Set(1);
    IIC_DELAY(FREQCTRL_DELAY);
    IIC_SDA_Set(1);
    IIC_DELAY(IOCTRL_DELAY);
}

/**************** PRIVATE FUNCTION *****************/
static inline void IIC_SDA_ConfigAsOutput(void)
{
    GPIO_InitTypeDef GPIO_Init;
    
    GPIO_Init.Pin = SDA_PIN;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SDA_GPIO, &GPIO_Init);    
}

static inline void IIC_SDA_ConfigAsInput(void)
{
    GPIO_InitTypeDef GPIO_Init;
    
    GPIO_Init.Pin = SDA_PIN;
    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SDA_GPIO, &GPIO_Init);    
}

static inline uint8_t IIC_SDA_Read(void)
{
    return (HAL_GPIO_ReadPin(SDA_GPIO, SDA_PIN) ? 1 : 0);
}

static inline void IIC_SDA_Set(uint8_t sta)
{
    HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, sta ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline void IIC_SCL_Set(uint8_t sta)
{
    HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, sta ? GPIO_PIN_SET : GPIO_PIN_RESET);
}



