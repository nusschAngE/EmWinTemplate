
#include "stm32h7xx.h"
#include "my_malloc.h"
#include "touch.h"
#include "delay.h"

/****************************************/

#define T_IIC_DELAY(x)      TimDelayUs(x)
#define T_IOCTRL_DELAY      (5)
#define T_FREQCTRL_DELAY    (2)

enum
{
    T_IIC_OK = 0,
    T_IIC_TIMEOUT,
    T_IIC_ERROR,
};

static void T_IIC_Init(void);
static void T_IIC_Start(void);
static void T_IIC_SendByte(uint8_t sVal);
static uint8_t T_IIC_WaitACK(void);
static uint8_t T_IIC_ReadByte(void);
static void T_IIC_SendACK(void);
static void T_IIC_SendNACK(void);
static void T_IIC_Stop(void);

/**************** PRIVATE FUNCTION*********************/

#define TOUCH_ADDRESS	(0x28)

/***  ***/
#define PRODUCT_ID      "9147" 

/*** register ***/
#define GT_CTRL_REG     (0x8040)//control
#define GT_CFG_REG      (0x8047)//config table
#define GT_CHK_REG      (0x80ff)//check sum
#define GT_ID_REG       (0x8140)//product ID

#define GT_GSTID_REG 	0x814e   	//GT9147 touch status
#define GT_TP1_REG 		0x8150  	//first point
#define GT_TP2_REG 		0x8158		//secont point
#define GT_TP3_REG 		0x8160		//third point
#define GT_TP4_REG 		0x8168		//fourth point
#define GT_TP5_REG 		0x8170		//fifth point

/***  ***/
static uint16_t ReadPointTBL[] = {GT_TP1_REG, GT_TP2_REG, GT_TP3_REG, GT_TP4_REG, GT_TP5_REG}; 

/*  GT9147 internal config
*/
const uint8_t GT9147_CFG_TBL[]=
{ 
	//0X60,0XE0,0X01,0X20,0X03,0X05,0X35,0X00,0X02,0X08,//trigger mode, falling edge
	0X60,0XE0,0X01,0X20,0X03,0X05,0X36,0X00,0X02,0X08,//polling mode, active low
	0X1E,0X08,0X50,0X3C,0X0F,0X05,0X00,0X00,0XFF,0X67,
	0X50,0X00,0X00,0X18,0X1A,0X1E,0X14,0X89,0X28,0X0A,
	0X30,0X2E,0XBB,0X0A,0X03,0X00,0X00,0X02,0X33,0X1D,
	0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X32,0X00,0X00,
	0X2A,0X1C,0X5A,0X94,0XC5,0X02,0X07,0X00,0X00,0X00,
	0XB5,0X1F,0X00,0X90,0X28,0X00,0X77,0X32,0X00,0X62,
	0X3F,0X00,0X52,0X50,0X00,0X52,0X00,0X00,0X00,0X00,
	0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,
	0X0F,0X03,0X06,0X10,0X42,0XF8,0X0F,0X14,0X00,0X00,
	0X00,0X00,0X1A,0X18,0X16,0X14,0X12,0X10,0X0E,0X0C,
	0X0A,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,0X00,0X29,0X28,0X24,0X22,0X20,0X1F,0X1E,0X1D,
	0X0E,0X0C,0X0A,0X08,0X06,0X05,0X04,0X02,0X00,0XFF,
	0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
	0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
	0XFF,0XFF,0XFF,0XFF,
}; 



/*  STATIC 
*/
static uint8_t GT9147_WriteRegister(uint16_t reg, uint8_t *sVal, uint8_t NumBytes);
static uint8_t GT9147_ReadRegister(uint16_t reg, uint8_t *rVal, uint8_t NumBytes);
static void GT9147_Send_Cfg(uint8_t mode);
static void GT9147_HardReset(void);
static uint8_t GT9147_Init(void);

/***************** PUBLIC FCUNTION *****************/

/* TP DEVICE */
TPPoints_t tpPoints;

/* device init */
uint8_t TP_Init(void)
{
	uint8_t ret = GT9147_Init();

	if(ret == 0)
	{
		//while(TP_PointChecked() != 0)
		//{
		//}
	}

    tpPoints.actPoints = 0;
    memset(tpPoints.xPos, 0, sizeof(tpPoints.xPos));
    memset(tpPoints.yPos, 0, sizeof(tpPoints.yPos));
	
    return (ret);
}

bool TP_ReadInterrupt(void)
{
	return (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_7) ? FALSE : TRUE);
}

uint8_t TP_PointChecked(void)
{
    uint8_t PointNbr = 0, ret = 0;
    uint8_t tmpReg = 0;

    GT9147_ReadRegister(GT_GSTID_REG, &tmpReg, 1);
    if(tmpReg & 0x80)
    {
        PointNbr = tmpReg & 0x0f;
        tmpReg = 0;
        ret = GT9147_WriteRegister(GT_GSTID_REG, &tmpReg, 1);
        if(ret != 0)
        {
        	//printf("tpchk write error\r\n");
        }
    }

    return (PointNbr);
}

/* @param point : 0 ~ TP_MAX_POINT-1
*/
uint8_t TP_ReadPoint(uint8_t point, uint16_t *xPos, uint16_t *yPos)
{
    uint8_t ret = 0;
    uint8_t tmpReg[4];

    if(point >= TP_MAX_POINT) {return (2);}

    memset(tmpReg, 0, 4);
    ret = GT9147_ReadRegister(ReadPointTBL[point], tmpReg, 4);
    if(ret == T_IIC_OK)
    {
        *xPos = ((uint16_t)tmpReg[1] << 8) + tmpReg[0];
        *yPos = ((uint16_t)tmpReg[3] << 8) + tmpReg[2];

        return (0);//error none
    }

    return (1);//error
}

void TP_ReadPoints(TPPoints_t *points)
{
    uint8_t ret = 0, i = 0;
    uint8_t tmp[4];

    points->actPoints = TP_PointChecked();
    if(points->actPoints)
    {
    	//printf(".");
        for(i = 0; i < points->actPoints; i++)
        {
            memset(tmp, 0, 4);
            ret = GT9147_ReadRegister(ReadPointTBL[i], tmp, 4);
            if(ret == T_IIC_OK)
            {
                points->xPos[i] = ((uint16_t)tmp[1] << 8) + tmp[0];
                points->yPos[i] = ((uint16_t)tmp[3] << 8) + tmp[2];
            }
        }        
    }

    for(i = points->actPoints; i < TP_MAX_POINT; i++)
    {
        points->xPos[i] = points->yPos[i] = 0;
    }
}

void TP_Release(TPPoints_t *points)
{
    uint8_t i = 0;

    points->actPoints = 0;
    for(i = 0; i < TP_MAX_POINT; i++)
    {
        points->xPos[i] = points->yPos[i] = 0;
    }
}

/**************** PRIVATE FUNCTION *********************/

/*  GT9147
*/
static uint8_t GT9147_WriteRegister(uint16_t reg, uint8_t *sVal, uint8_t NumBytes)
{
    uint8_t tmp;

	T_IIC_Start();
	T_IIC_SendByte(TOUCH_ADDRESS);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* reg msb */
	tmp = (uint8_t)((reg & 0xff00) >> 8);
	T_IIC_SendByte(tmp);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* reg lsb */
	tmp = (uint8_t)(reg & 0x00ff);
	T_IIC_SendByte(tmp);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* send data */
	while(NumBytes--)
	{
		T_IIC_SendByte(*sVal);
		if(T_IIC_WaitACK())	
		    return (T_IIC_TIMEOUT);
		/* next data */
		sVal++;
	}
	T_IIC_Stop();
	
	return (T_IIC_OK);
}


static uint8_t GT9147_ReadRegister(uint16_t reg, uint8_t *rVal, uint8_t NumBytes)
{
    uint8_t tmp;

	T_IIC_Start();
	T_IIC_SendByte(TOUCH_ADDRESS);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* reg msb */
	tmp = (uint8_t)((reg & 0xff00) >> 8);
	T_IIC_SendByte(tmp);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* reg lsb */
	tmp = (uint8_t)(reg & 0x00ff);
	T_IIC_SendByte(tmp);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT); 
	/* restart */
	T_IIC_Stop();
	T_IIC_Start();
	/* read sequence */
	T_IIC_SendByte(TOUCH_ADDRESS + 1);
	if(T_IIC_WaitACK())	
	    return (T_IIC_TIMEOUT);
	/* read data */
	while(NumBytes--)
	{
		*rVal = T_IIC_ReadByte();
		rVal++;

		if(NumBytes)
			T_IIC_SendACK();
		else
			T_IIC_SendNACK();
	}
	T_IIC_Stop();

	return (T_IIC_OK);
}

/*  mode : 0->not save, 1->save to flash
*/
static void GT9147_Send_Cfg(uint8_t mode)
{
	uint8_t buf[2];
	uint8_t i = 0;
	buf[0] = 0;
	buf[1] = mode;

	/* get checksum */
	for(i = 0; i < sizeof(GT9147_CFG_TBL); i++)
	{
	    buf[0] += GT9147_CFG_TBL[i];
	}   
    buf[0] = (~buf[0]) + 1;

	GT9147_WriteRegister(GT_CFG_REG, (uint8_t*)GT9147_CFG_TBL, sizeof(GT9147_CFG_TBL));//send config table
	GT9147_WriteRegister(GT_CHK_REG, buf, 2);//send checksum and config mode
} 


/* GPIO Init */
static void GT9147_HardReset(void)
{
	GPIO_InitTypeDef GPIO_Init;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	/* reset pin */
	GPIO_Init.Pin = GPIO_PIN_8;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOI,&GPIO_Init);

	/* INT detect pin */
	GPIO_Init.Pin = GPIO_PIN_7;
	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Pull = GPIO_PULLUP;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOH,&GPIO_Init);

	/* reset on */
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_RESET);
	TimDelayMs(10);
	/* reset high, reset sequence done */
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_SET);
	TimDelayMs(10);
	
	/* set INT pin input */
	GPIO_Init.Pin = GPIO_PIN_7;
	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOH,&GPIO_Init);
	TimDelayMs(100);
}

static uint8_t GT9147_Init(void)
{
	uint8_t tmp[5];

	T_IIC_Init();
	GT9147_HardReset();
	//TimDelayMs(5);

	myMemset(tmp, 0, 5);
    /* read product ID */
    GT9147_ReadRegister(GT_ID_REG, tmp, 4);
    //printf("id tmp = %s\r\n", tmp);
    if(myStrcmp((const char*)tmp, PRODUCT_ID) != 0)
    {
        printf("Touch's Product ID read error!\r\n");
        return (1);
    }
    /* software reset */
    tmp[0] = 0x02;
    GT9147_WriteRegister(GT_CTRL_REG, tmp, 1);
    /* update config table */
    GT9147_ReadRegister(GT_CFG_REG, tmp, 1);
    if(tmp[0] < 0x60)
    {//if current cfg version < 0x60, update and save
        GT9147_Send_Cfg(1);
    }
    /* init done */
    TimDelayUs(20);
    tmp[0] = 0x00;
    GT9147_WriteRegister(GT_CTRL_REG, tmp, 1);

    //printf("GT9147 init done!\r\n");

    return (0);//ERROR NONE
}

/*  IIC function 
*/
static inline void T_SDA_ConfigAsInput(void)
{
	GPIO_InitTypeDef GPIO_Init;

	GPIO_Init.Pin = GPIO_PIN_3;
    GPIO_Init.Mode = GPIO_MODE_INPUT; 
    GPIO_Init.Pull = GPIO_PULLUP;        
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  
    HAL_GPIO_Init(GPIOI, &GPIO_Init); 
}

static inline void T_SDA_ConfigAsOutput(void)
{
	GPIO_InitTypeDef GPIO_Init;

	GPIO_Init.Pin = GPIO_PIN_3;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP; 
    GPIO_Init.Pull = GPIO_PULLUP;        
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  
    HAL_GPIO_Init(GPIOI, &GPIO_Init); 
}

static inline void T_SDA_Set(uint8_t sta)
{
	if(sta)
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);
}

static inline uint8_t T_SDA_Read(void)
{
	return HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_3) ? 1 : 0;
}

static inline void T_SCL_Set(uint8_t sta)
{
	if(sta)
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);
}

static void T_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Init;
    __HAL_RCC_GPIOH_CLK_ENABLE();		
	__HAL_RCC_GPIOI_CLK_ENABLE();		

    /* SCL */
    GPIO_Init.Pin = GPIO_PIN_6;          
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP; 
    GPIO_Init.Pull = GPIO_PULLUP;        
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  
    HAL_GPIO_Init(GPIOH, &GPIO_Init);    
	/* SDA */
    GPIO_Init.Pin = GPIO_PIN_3;         
    HAL_GPIO_Init(GPIOI, &GPIO_Init);    
}

/*  IIC START : when CLK is high, change SDA from high to low
*/
static void T_IIC_Start(void)
{	
    T_SCL_Set(1);
    T_SDA_Set(1);
    T_SDA_ConfigAsOutput();
    T_IIC_DELAY(T_IOCTRL_DELAY);
    T_SDA_Set(0);
    T_IIC_DELAY(T_FREQCTRL_DELAY);
    T_SCL_Set(0);
}

static void T_IIC_SendByte(uint8_t sVal)
{
    uint8_t t;   
    
    T_SCL_Set(0);
	T_SDA_ConfigAsOutput(); 	    
	T_IIC_DELAY(T_IOCTRL_DELAY);
	for(t = 0; t < 8; t++)
    {
		T_SDA_Set(sVal & 0x80);//MSB first
		sVal <<= 1; 		
		
		T_SCL_Set(1); 
		T_IIC_DELAY(T_FREQCTRL_DELAY);
		T_SCL_Set(0);	
		T_IIC_DELAY(T_FREQCTRL_DELAY);
    }
}

static uint8_t T_IIC_WaitACK(void)
{
    uint8_t cnt = 0;
	
    T_SDA_Set(1);
    T_SDA_ConfigAsInput();    
    T_SCL_Set(1);
    T_IIC_DELAY(T_IOCTRL_DELAY);
    while(T_SDA_Read())
    {
        if(++cnt > 250)
        {
            T_IIC_Stop();
            return (1);
        }
        T_IIC_DELAY(T_FREQCTRL_DELAY);
    }

    return (0);
}

static uint8_t T_IIC_ReadByte(void)
{
	uint8_t i, receive = 0;

	T_SCL_Set(0); 
 	T_SDA_ConfigAsInput();
	T_IIC_DELAY(T_IOCTRL_DELAY);
	for(i = 0; i < 8; i++ )
	{ 
		T_SCL_Set(0); 	    	   
		T_IIC_DELAY(T_FREQCTRL_DELAY);
		T_SCL_Set(1);	 
		T_IIC_DELAY(T_FREQCTRL_DELAY);
		receive <<= 1;
		if(T_SDA_Read())
		{
		    receive |= 0x01;   
		}
	}	
	    
 	return (receive);
}

static void T_IIC_SendACK(void)
{
	T_SCL_Set(0);
	T_IIC_DELAY(T_FREQCTRL_DELAY);
	T_SDA_Set(0);
	T_SDA_ConfigAsOutput();	
	T_IIC_DELAY(T_IOCTRL_DELAY);
	T_SCL_Set(1);
	T_IIC_DELAY(T_FREQCTRL_DELAY);
}

static void T_IIC_SendNACK(void)
{
	T_SCL_Set(0);
	T_IIC_DELAY(T_FREQCTRL_DELAY);
	T_SDA_Set(1);
	T_SDA_ConfigAsOutput();	
	T_IIC_DELAY(T_IOCTRL_DELAY);
	T_SCL_Set(1);
	T_IIC_DELAY(T_FREQCTRL_DELAY);
}


/*  IIC STOP : when CLK is high, change SDA from low to high
*/
static void T_IIC_Stop(void)
{	
    T_SCL_Set(0);
    //T_IIC_DELAY(2);
    T_SDA_Set(0);    
    T_SDA_ConfigAsOutput();
    T_IIC_DELAY(T_IOCTRL_DELAY);

    T_SCL_Set(1);
    T_IIC_DELAY(T_FREQCTRL_DELAY);
    T_SDA_Set(1);
    T_IIC_DELAY(T_IOCTRL_DELAY);
}


