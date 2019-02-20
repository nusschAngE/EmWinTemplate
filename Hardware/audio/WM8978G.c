
#include "stm32h7xx.h"

#include "delay.h"
#include "sys_i2c.h"

#include "audio_api.h"
#include "WM8978G.h"

/* address */
#define WM8978_ADDR             (0x34)//without R/W bit

/* IIC secquence
*  START --- ADDR --- (REG<<1)|(VALUE[1]:bit8) --- VALUE[0]:bit7~0 --- STOP
*/

/* register */
static uint16_t WM8978_REG[58] = 
{
	0x0000,0x0000,0x0000,0x0000,0x0050,0x0000,0x0140,0x0000,
	0x0000,0x0000,0x0000,0x00FF,0x00FF,0x0000,0x0100,0x00FF,
	0x00FF,0x0000,0x012C,0x002C,0x002C,0x002C,0x002C,0x0000,
	0x0032,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0038,0x000B,0x0032,0x0000,0x0008,0x000C,0x0093,0x00E9,
	0x0000,0x0000,0x0000,0x0000,0x0003,0x0010,0x0010,0x0100,
	0x0100,0x0002,0x0001,0x0001,0x0039,0x0039,0x0039,0x0039,
	0x0001,0x0001
};

/*  STATIC
*/
static uint8_t WM8978_WriteRegister(uint8_t reg, uint16_t val);
static uint8_t WM8978_ReadRegister(uint8_t reg, uint16_t *val);
static uint8_t WM8978_SetDACEN(uint8_t on);
static uint8_t WM8978_SetADCEN(uint8_t on);
static uint8_t WM8978_OutputConfig(uint8_t on, uint8_t bypass);
static uint8_t WM8978_MIC_Gain(uint8_t gain);
static uint8_t WM8978_LineIN_Gain(uint8_t gain);
static uint8_t WM8978_AUX_Gain(uint8_t gain);
static uint8_t WM8978_InputConfig(uint8_t chan, uint8_t on);
static uint8_t WM8978_SetHPVol(uint8_t vol);
static uint8_t WM8978_SetSPKVol(uint8_t vol);
static uint8_t WM8978_I2SConfig(uint8_t protocal, uint32_t sampleRate, uint8_t dataSize);

/* param
*/
static uint8_t amp_CurrVolume = 0;
static uint8_t amp_CurrMute = 0;
//static uint8_t amp_CurrEQ = 0;
static uint8_t amp_OutputChannel = 0;
//static uint8_t amp_InputChannel = 0;

/******************* PUBLIC FUNCTION *******************/
uint8_t Audio_AmpInit(void)
{
    uint8_t ret = 0;

    SYSI2C_Init();
    /* WM8978 Init */
	ret = WM8978_WriteRegister(0, 0);//reset
	if(ret != 0) {
	    return (1);
	}
	
    TimDelayMs(100);
    
	/* normal init */
	WM8978_WriteRegister(52, 0);
	WM8978_WriteRegister(53, 0);
	WM8978_WriteRegister(54, 0);
	WM8978_WriteRegister(55, 0);
	
	WM8978_WriteRegister(1, 0x1b);
	WM8978_WriteRegister(2, 0x1b0);//LOUT1, ROUT1, BOOSTEN
	WM8978_WriteRegister(3, 0x6c); //LOUT2, ROUT2, MIXEN
	WM8978_WriteRegister(6, 0);	   //MCLK from master
	WM8978_WriteRegister(43, 0x10);
	WM8978_WriteRegister(47, 0x100);
	WM8978_WriteRegister(48, 0x100);
	WM8978_WriteRegister(49, 0x03);
	WM8978_WriteRegister(10, 0x80);
	WM8978_WriteRegister(14, 0x80);

    /* decode start */
    WM8978_SetDACEN(TRUE);
    /* param init */
    amp_CurrVolume = 0;
    amp_CurrMute = 0;
    //amp_CurrEQ = 0;
    amp_OutputChannel = 0;
    //amp_InputChannel = 0;

    return (0);
}

uint8_t Audio_AmpSetMute(bool mute)
{
	uint8_t ret = 0;

    amp_CurrMute = mute;
	if(amp_CurrMute)
	{
		if(amp_OutputChannel & CODEC_OUTPUT_HP)
			ret = WM8978_SetHPVol(0);
		if(amp_OutputChannel & CODEC_OUTPUT_SPK)
			ret = WM8978_SetSPKVol(0);
	}
	else
	{
		if(amp_OutputChannel & CODEC_OUTPUT_HP)
			ret = WM8978_SetHPVol(amp_CurrVolume);
		if(amp_OutputChannel & CODEC_OUTPUT_SPK)
			ret = WM8978_SetSPKVol(amp_CurrVolume);
	}

	if(ret != 0) {
	    return (1);
	}
	    
    /* error none */
    return (0);
}

/* @param channel : CODEC_OUTPUT_
*/
uint8_t Audio_AmpSetChannel(uint8_t ch)
{
	uint8_t ret = 0;

    amp_OutputChannel = ch;
    switch (amp_OutputChannel)
    {
        case CODEC_OUTPUT_HP:
			ret += WM8978_SetHPVol(amp_CurrVolume);
			ret += WM8978_SetSPKVol(0);
			break;
	    case CODEC_OUTPUT_SPK:
			ret += WM8978_SetSPKVol(amp_CurrVolume);
			ret += WM8978_SetHPVol(0);
			break;
	    case CODEC_OUTPUT_ALL:
			ret += WM8978_SetSPKVol(amp_CurrVolume);
			ret += WM8978_SetHPVol(amp_CurrVolume);
			break;
	    case CODEC_OUTPUT_NONE:
	        ret += WM8978_SetSPKVol(0);
			ret += WM8978_SetHPVol(0);
			break;
	    default:
	        return (1);
    }
        
    if(ret != 0) {
        return (2);
    }
    
    /* error none */
    return (0);
}   

/* @para vol:0~63
*/
uint8_t Audio_AmpSetVolume(uint8_t vol)
{
	uint8_t ret = 0;

	amp_CurrVolume = vol;
    if(amp_OutputChannel & CODEC_OUTPUT_HP)
    {
    	ret = WM8978_SetHPVol(vol);
    }

    if(amp_OutputChannel & CODEC_OUTPUT_SPK)
    {
    	ret = WM8978_SetSPKVol(vol);
    }

    if(ret != 0) {
        return (1);
    }
        
    /* error none */
	return (0);
}

uint8_t Audio_AmpSetEQ(uint8_t eq)
{
    //amp_CurrEQ = eq;
    return (0);
}

/* @param fmt : AUD_PROTOCAL_ [0-LSB, 1-MSB, 2-I2S/PCM, 3-PCM/DSP]
*  @param sampleRate : 
*  @param dataSize : AUD_DATASIZE_ [0-16bits, 1-20bits, 2-24bits, 3-32bits]
*/
uint8_t Audio_AmpI2SConfig(uint8_t protocal, uint32_t sampleRate, uint8_t dataSize)
{
    uint8_t ret = 0;

    ret = WM8978_I2SConfig(protocal, sampleRate, dataSize);
    if(ret != 0) {
        return (1);
    }
        
    /* error none */
    return (0);
}

uint8_t Audio_AmpPlayStart(void)
{
	uint8_t ret = 0;

    ret += WM8978_OutputConfig(TRUE, TRUE);
    if(ret != 0) {
        return (1);
    }
        
    /* error none */
    return (0);
}

uint8_t Audio_AmpPlayStop(void)
{
    uint8_t ret = 0;

    ret += WM8978_OutputConfig(FALSE, TRUE);
    if(ret != 0) {
        return (1);
    }
        
    /* error none */
    return (0);
}

uint8_t Audio_DecoderStart(void)
{
    uint8_t ret = 0;

    ret += WM8978_SetDACEN(TRUE);
    if(ret != 0) {
        return (1);
    }
        
    /* error none */
    return (0);
}

uint8_t Audio_DecoderStop(void)
{
    uint8_t ret = 0;

    ret += WM8978_SetDACEN(FALSE);
    if(ret != 0) {
        return (1);
    }
        
    /* error none */
    return (0);
}

/******************* PRIVATE FUNCTION *******************/
static uint8_t WM8978_WriteRegister(uint8_t reg, uint16_t val)
{
    uint8_t tmp, ret;

	SYSI2C_Start();
    /* addr */
    tmp = WM8978_ADDR;
    SYSI2C_SendByte(tmp);
    ret = SYSI2C_WaitACK();
    if(ret == 1)//nack
    {
        return (1);
    }
    /* control byte 1 */
    tmp = (reg << 1) | val >> 8;
    SYSI2C_SendByte(tmp);
    ret = SYSI2C_WaitACK();
    if(ret == 1)//nack
    {
        return (1);
    }
    /* control byte 0 */
    tmp = val & 0xff;
    SYSI2C_SendByte(tmp);
    ret = SYSI2C_WaitACK();
    if(ret == 1)//nack
    {
        return (1);
    }
    /* Error None */
    SYSI2C_Stop();
	/* update backup */
    WM8978_REG[reg] = val;
    
    return(0);
}

static uint8_t WM8978_ReadRegister(uint8_t reg, uint16_t *val)
{
	*val = WM8978_REG[reg];
	
	return (0);
}

static uint8_t WM8978_SetDACEN(uint8_t on)
{
	uint16_t tmpVal = 0;

	WM8978_ReadRegister(3, &tmpVal);
	
	if(on) {tmpVal |= (3<<0);}
	else   {tmpVal &= ~(3<<0);}

	return WM8978_WriteRegister(3, tmpVal);
}

static uint8_t WM8978_SetADCEN(uint8_t on)
{
	uint16_t tmpVal = 0;

	WM8978_ReadRegister(2, &tmpVal);
	
	if(on) {tmpVal |= (3<<0);}
	else   {tmpVal &= ~(3<<0);}

	return WM8978_WriteRegister(2, tmpVal);
	
}

static uint8_t WM8978_OutputConfig(uint8_t on, uint8_t bypass)
{
	uint16_t tmpVal = 0;
	uint8_t ret = 0;

	if(on) {tmpVal |= 1<<0;}//dac en

	if(bypass)
	{
		tmpVal |= 1<<1;
		tmpVal |= 5<<2;
	}

	ret += WM8978_WriteRegister(50, tmpVal);
	ret += WM8978_WriteRegister(51, tmpVal);
	
	if(ret != 0)
	    return (1);
    else
	    return (0);
}

/* fmt : 0-LSB, 1-MSB, 2-IIS/PCM, 3-PCM/DSP
*  sampleRate : 
*  dataWidth : data size, 0-16bits, 1-20bits, 2-24bits, 3-32bits
*/
static uint8_t WM8978_I2SConfig(uint8_t protocal, uint32_t sampleRate, uint8_t dataSize)
{    
    (void)sampleRate;

	if(protocal > 3) {return (1);}
	if(dataSize > 3) {return (1);}

    return WM8978_WriteRegister(4, (protocal<<3)|(dataSize<<5));
}

//0~63, 0.75db/step. -12 ~ 35.25db
static uint8_t WM8978_MIC_Gain(uint8_t gain)
{
	uint8_t ret = 0;

	if(gain > 63) {return (1);}

	ret += WM8978_WriteRegister(45, gain);
	ret += WM8978_WriteRegister(46, gain | (1<<8));
	
	if(ret != 0)
	    return (1);
    else
	    return (0);
}

//0 : LineIN OFF, 1~7 : LineIN ON,3db/step. -12 ~ 6db
static uint8_t WM8978_LineIN_Gain(uint8_t gain)
{
	uint16_t tmpVal;
	uint8_t ret = 0;

	if(gain > 7) {return (1);}
	
	WM8978_ReadRegister(47, &tmpVal);
	tmpVal &= ~(7<<4);//clear
	ret += WM8978_WriteRegister(47, tmpVal|(gain<<4));
	
	WM8978_ReadRegister(48, &tmpVal);
	tmpVal &= ~(7<<4);//clear
	ret += WM8978_WriteRegister(48, tmpVal|(gain<<4));
	
	if(ret != 0) 
	    return (1);
    else
	    return (0);
}

//0 : AUX OFF, 1~7 : AUX ON 3db/step. -12 ~ 6db
static uint8_t WM8978_AUX_Gain(uint8_t gain)
{
	uint16_t tmpVal;
	uint8_t ret = 0;

	if(gain > 7) {return (1);}
	
	WM8978_ReadRegister(47, &tmpVal);
	tmpVal &= ~(7<<0);//clear
	ret += WM8978_WriteRegister(47, tmpVal|(gain<<0));
	
	WM8978_ReadRegister(48, &tmpVal);
	tmpVal &= ~(7<<0);//clear
	ret += WM8978_WriteRegister(48, tmpVal|(gain<<0));
	
	if(ret != 0)    
	    return (1);
    else
	    return (0);
}


static uint8_t WM8978_InputConfig(uint8_t chan, uint8_t on)
{
	uint16_t tmpVal;
	uint8_t ret = 0;

	if(chan & CODEC_INPUT_MIC)
	{
		//INPPGAR, INPPGAL
		WM8978_ReadRegister(2, &tmpVal);
		
		if(on) {tmpVal |= (3<<2);}
		else   {tmpVal &= ~(3<<2);}
		
		ret += WM8978_WriteRegister(2, tmpVal);
		//LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA
		WM8978_ReadRegister(44, &tmpVal);
		
		if(on) {tmpVal |= 3<<4 | 3<<0;}
		else   {tmpVal &= ~(3<<4 | 3<<0);}
		
		ret += WM8978_WriteRegister(44, tmpVal);
		
		if(ret != 0) {return (1);}
	}

	if(chan & CODEC_INPUT_LINEIN)
	{
		if(on) {ret = WM8978_LineIN_Gain(5);}//
		else   {ret = WM8978_LineIN_Gain(0);}
		
		if(ret != 0) {return (1);}
	}

	if(chan & CODEC_INPUT_AUX)
	{
		if(on) {ret = WM8978_AUX_Gain(7);}//
		else   {ret = WM8978_AUX_Gain(0);}
		
		if(ret != 0) {return (1);}
	}

	return (0);
}

// 0 ~ 63
static uint8_t WM8978_SetHPVol(uint8_t vol)
{
	uint8_t ret = 0;

	if(vol > 63) {return (1);}
	if(vol == 0) {vol |= (1<<6);}//mute
		
	ret += WM8978_WriteRegister(52, vol);
	ret += WM8978_WriteRegister(53, vol|(1<<8));

	if(ret != 0)
	    return (1);
    else
	    return (0);
}

// 0 ~ 63
static uint8_t WM8978_SetSPKVol(uint8_t vol)
{
	uint8_t ret = 0;

	if(vol > 63) {return (1);}
	if(vol == 0) {vol |= (1<<6);}//mute
		
	ret += WM8978_WriteRegister(54, vol);
	ret += WM8978_WriteRegister(55, vol|(1<<8));

	if(ret != 0) 
	    return (1);
    else
	    return (0);
}


