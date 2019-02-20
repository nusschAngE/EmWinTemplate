
#include "stm32h7xx.h"
#include "qspi_flash.h"
#include "delay.h"

/********* W25Qxxx command ***********/
/* write enable/disable cmd */
#define W25Q_WRITE_ENABLE     0x06
#define W25Q_WRITE_ENVSR      0x50//volatile status register
#define W25Q_WRITE_DISABLE    0x04
/* read status register */
#define W25Q_READ_SR1         0x05
#define W25Q_READ_SR2         0x35
#define W25Q_READ_SR3         0x15
/* write status register */
#define W25Q_WRITE_SR1        0x01
#define W25Q_WRITE_SR2        0x31
#define W25Q_WRITE_SR3        0x11
/* read id */
#define W25Q_READID           0x90
#define W25Q_READID_DSPI      0x92
#define W25Q_READID_QSPI      0x94
/* QSPI Enable &Disable */
#define W25Q_QSPI_ENABLE      0x38
#define W25Q_QSPI_DISABLE     0xff
/* address mode */
#define W25Q_ADDR_3BYTE       0xe9
#define W25Q_ADDR_4BYTE       0xb7
/* fast read */
#define W25Q_FAST_READ        0x0b
#define W25Q_FAST_READ4B      0x0c
/* page program */
#define W25Q_PAGE_PROGRAM     0x02
/* sector erase */
#define W25Q_SECTOR_ERASE     0x20
/* set read parameter */
#define W25Q_READ_PARA        0xc0
/* reset */
#define W25Q_RESET_ENABLE	  0x66
#define W25Q_RESET			  0x99

/* QSPI interface state */
enum
{
    QSPI_OK          = 0,
    QSPI_ERR_CMD     = 1,
    QSPI_ERR_READ    = 2,
    QSPI_ERR_WRITE   = 3,

    QSPI_ERR,
};

/* QSPI Flash using SPI mode */
enum
{
    SerialSPI     = 0,
    SerialDUALSPI,
    SerialQUADSPI,
};

/*  STATIC
*/
static void QSPI_IOInit(void);
static uint8_t QSPI_SendCommand(uint32_t Inst, uint32_t Addr, 
                                uint32_t dCycles, uint32_t InstMode, 
                                uint32_t AddrMode, uint32_t AddrSize, 
                                uint32_t DataMode);
static uint8_t QSPI_Transmit(uint8_t *buff, uint32_t size);   
static uint8_t QSPI_Receive(uint8_t *buff, uint32_t size);

static uint8_t W25Q_SoftReset(uint8_t itface);
static bool W25Q_WriteEnable(uint8_t itface);
static uint8_t W25Q_WriteDisable(uint8_t itface);
static uint8_t W25Q_ReadSR(uint32_t SRxRCmd, uint8_t itface);
static uint8_t W25Q_WriteSR(uint32_t SRxWCmd, uint8_t Val, uint8_t itface);

static uint8_t W25Q_SetAddressMode(uint32_t AddrModeCmd);
static uint8_t W25Q_SetReadParameter(uint8_t dummy, uint8_t wrap);
static uint16_t W25Q_ReadDeviceID(void);
static uint8_t W25Q_EnterQuadMode(void);
static uint8_t W25Q_ExitQuadMode(void);
static bool W25Q_ChkFinished(uint32_t timeout);
static uint8_t W25Q_EraseSector(uint32_t addr);
static uint8_t W25Q_PageProgram(uint8_t *buff, uint32_t addr, uint16_t size);
static uint8_t W25Q_Read(uint8_t *buff, uint32_t addr, uint16_t size);
//static uint8_t W25Qxx_WriteSector(uint8_t *pBuff, uint32_t Addr, uint16_t wSize, uint8_t check);

static uint8_t OPRBUFFER[QFL_SECTORSIZE];
static uint8_t QFLSerial = SerialSPI;
static QSPI_HandleTypeDef QSPI_Handler;

/******************* PUBLIC FUNCTION ***************************/

/* w25qxxx init */
uint8_t QFL_Init(void)
{
    uint16_t DeviceID = 0;
    uint8_t ret = 0;
    
    /* QSPI initialize */
    QSPI_IOInit();
    
    /* enable QSPI mode */
    ret = W25Q_EnterQuadMode();
    if(ret != 0) {
        printf("Enter Quad Mode, ret = %d\r\n", ret);
        return (1);
    }
    
    /* get flash id */
    DeviceID = W25Q_ReadDeviceID();
    if(DeviceID != 0xEF18) {
        printf("Read ID, DeviceID = 0x%04x\r\n", DeviceID);
        return (2);
    }

    /* set 4byte address mode */
    ret = W25Q_SetAddressMode(W25Q_ADDR_4BYTE);
    /* set read parameter */
    ret += W25Q_SetReadParameter(3, 0);
    if(ret != 0) {
        printf("Deveice Config Error, ret = %d\r\n", ret);
        return (3);
    }

    return (0);
}

uint8_t QFL_EraseSector(uint32_t addr)
{
	return W25Q_EraseSector(addr);
}

uint8_t QFL_EraseSectors(uint32_t StartSector, uint32_t SectorNbr)
{
    uint8_t ret;
    uint32_t currSector = StartSector;

    while(SectorNbr)
    {
        ret = W25Q_EraseSector(currSector * QFL_SECTORSIZE);
        if(ret != 0) {
            printf("QFL Erase Sector[%d], ret = %d\r\n", currSector, ret);
            return (1);
        }
            
        currSector++;
        SectorNbr--;
    }

    return (0);
}

/* SPI flash read */
uint8_t QFL_Read(uint8_t *buff, uint32_t addr, uint16_t size)
{
    uint8_t ret = 0;

    ret = W25Q_Read(buff, addr, size);
    if(ret != 0) {
        printf("QFL Read[%d], ret = %d\r\n", addr, ret);
        return (1);
    }
    
    return (0);
}

/* bytes : number bytes one cycle */
uint8_t QFL_ReadWithCycle(cycle cycleFunc, uint32_t addr, uint16_t size)
{
    uint32_t addr_reg = READ_REG(QSPI_Handler.Instance->AR);
    __IO uint32_t *data_reg = &QSPI_Handler.Instance->DR;

    uint16_t timeout = 0xffff;
    uint8_t ret = QSPI_ERR;

    if(cycleFunc == NULL) {
    	return (1);
   	}

	/* send read command */
	if(QFLSerial == SerialSPI)
	{
	    ret = QSPI_SendCommand(W25Q_FAST_READ, addr, 
                             	8, QSPI_INSTRUCTION_1_LINE, 
                             	QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_32_BITS, 
                               	QSPI_DATA_1_LINE);
	}
	else if(QFLSerial == SerialQUADSPI)
	{
        ret = QSPI_SendCommand(W25Q_FAST_READ, addr, 
                             	8, QSPI_INSTRUCTION_4_LINES, 
                             	QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_32_BITS, 
                               	QSPI_DATA_4_LINES);
    }
    /* send command error */ 
 	if(ret != QSPI_OK) {
		return (2);
    }

    QSPI_Handler.Instance->DLR = size - 1;
    /* Configure QSPI: CCR register with functional as indirect read */
    MODIFY_REG(QSPI_Handler.Instance->CCR, QUADSPI_CCR_FMODE, QUADSPI_CCR_FMODE_0);
    /* Start the transfer by re-writing the address in AR register */
    WRITE_REG(QSPI_Handler.Instance->AR, addr_reg);

    while(size)
    {
        if(__HAL_QSPI_GET_FLAG(&QSPI_Handler, QSPI_FLAG_FT|QSPI_FLAG_TC) != RESET)
        {
            cycleFunc(*(__IO uint8_t *)data_reg);

            timeout = 0xffff;
            size--;
        }
        /* chekc if timeout? */
        DelayUs(2);
        if(--timeout == 0)  {
            return (3);
        }
    }

    timeout = 0xffff;
    while(timeout--)
    {
        if(__HAL_QSPI_GET_FLAG(&QSPI_Handler, QSPI_FLAG_TC) != RESET)
        {
            __HAL_QSPI_CLEAR_FLAG(&QSPI_Handler, QSPI_FLAG_TC);
        }
        DelayUs(5);
    }

    return (0);
}

/*  SPI Flash write, not check 
*   page auto shift
*/
uint8_t QFL_WriteNoChk(uint8_t *buff, uint32_t addr, uint32_t size)
{
    uint8_t ret;
    uint16_t curWriteSize, remainPageSize;

    //0 ~ SPIFLASH_MEMSIZE-1
    if((addr + size) > QFL_MEMSIZE) {
        return (1);
    }
    /* init value 
    */
    // remian bytes in current page
    remainPageSize = QFL_PAGESIZE - addr % QFL_PAGESIZE;
    // current write size
    curWriteSize = (size <= remainPageSize) ? (size) : (remainPageSize);

    while(size)
    {
        ret = W25Q_PageProgram(buff, addr, curWriteSize);
        if(ret != 0) {
            return (2);
        }

        buff += curWriteSize;
        addr += curWriteSize;
        size -= curWriteSize;
        /* next tune */
        if(size > QFL_PAGESIZE) {
            curWriteSize = QFL_PAGESIZE;
        } else {
            curWriteSize = size;
		}
    }

    return (0);
}

/*  SPI Flash write with check
*   page auto shift
*   sector check and erase
*/
uint8_t QFL_WriteWithChk(uint8_t *buff, uint32_t addr, uint32_t size)
{
    uint8_t ret;
    //page program value
    uint16_t curWriteSize;
    //sector check value
    uint32_t curSector, SectorOfs, remainSectorSize;
    uint16_t i, NeedErase = 0;
    
    //0 ~ SPIFLASH_MEMSIZE-1
    if((addr + size) > QFL_MEMSIZE) {
        return (1);
    }

    /* init value */
    // start sector : 0 ~ SPIFLASH_SECTORNBR-1
    curSector = addr / QFL_SECTORSIZE;
    // offset in current sector
    SectorOfs = addr % QFL_SECTORSIZE;
    // remain bytes in current sector
    remainSectorSize = QFL_SECTORSIZE - SectorOfs;
	// current write size
	curWriteSize = (size <= remainSectorSize) ? (size) : (remainSectorSize);

    while(size)
    {
    	//printf("size = %d, ofs = %d\r\n", curWriteSize, SectorOfs);
        // read current sector
        ret = W25Q_Read(OPRBUFFER, curSector * QFL_SECTORSIZE, QFL_SECTORSIZE);
        if(ret != 0) {
            return (2);
        }

        // check
        for(i = 0; i < curWriteSize; i++)
        {
        	//printf("%d,", OPRBUFFER[i + SectorOfs]);
            if(OPRBUFFER[i + SectorOfs] != 0xFF)
            {
            	NeedErase = 1;
                break;
            }
        }

        // need erase
        if(NeedErase)
        {
        	printf("sector[%d] need erase\r\n", curSector);
            ret = W25Q_EraseSector(curSector * QFL_SECTORSIZE);
            if(ret != 0) {
                return (3);
			}
	
            for(i = 0; i < curWriteSize; i++) {
                OPRBUFFER[i + SectorOfs] = buff[i];
            }

            ret = QFL_WriteNoChk(OPRBUFFER, curSector * QFL_SECTORSIZE, QFL_SECTORSIZE);
            if(ret != 0) {
                return (4);
           	}
        }
        else
        {
            ret = QFL_WriteNoChk(buff, addr, curWriteSize);
            if(ret != 0) {
                return (5);
            }
        }

        curSector += 1;
        SectorOfs = 0;//init state process done, set byte off to 0
        NeedErase = 0;

        buff += curWriteSize;
        addr += curWriteSize;
        size -= curWriteSize;
        /* new write size */
        if(size > QFL_SECTORSIZE) {
            curWriteSize = QFL_SECTORSIZE;
        } else {
            curWriteSize = size;
		}
    }

    return (0);
}

/******************** PUBLIC FUNCTION ********************/
/*
*   QSPI initialize
**/
static void QSPI_IOInit(void)
{
    GPIO_InitTypeDef GPIO_Init;
    /* Clock Enable */
    __HAL_RCC_QSPI_CLK_ENABLE();       
    __HAL_RCC_GPIOB_CLK_ENABLE();      
    __HAL_RCC_GPIOF_CLK_ENABLE();      
    
    GPIO_Init.Pin = GPIO_PIN_6;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;        
    GPIO_Init.Pull = GPIO_PULLUP;              
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;  
    GPIO_Init.Alternate = GPIO_AF10_QUADSPI;   
    HAL_GPIO_Init(GPIOB, &GPIO_Init);
    
    GPIO_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL;              
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;   
    GPIO_Init.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_Init);
    
    GPIO_Init.Pin = GPIO_PIN_2;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL; 
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF9_QUADSPI;   
    HAL_GPIO_Init(GPIOB, &GPIO_Init);
    
    GPIO_Init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL; 
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_Init);

    /* QSPI Init */
    QSPI_Handler.Instance = QUADSPI;
    QSPI_Handler.Init.ClockPrescaler = 1; //QSPI clock frequency -> 200/(CP + 1) = 100mhz
    QSPI_Handler.Init.FifoThreshold = 8;
    QSPI_Handler.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;//must set 0 in DDR mode
    QSPI_Handler.Init.FlashSize = (QFL_MEMSIZE - 1);//32Mbyte
    QSPI_Handler.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;//55ns
    QSPI_Handler.Init.ClockMode = QSPI_CLOCK_MODE_0;
    QSPI_Handler.Init.FlashID = QSPI_FLASH_ID_1;
    QSPI_Handler.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
    HAL_QSPI_Init(&QSPI_Handler);
}

/*
*   QSPI send Command
*/
static uint8_t QSPI_SendCommand(uint32_t Inst, uint32_t Addr, 
                                uint32_t dCycles, uint32_t InstMode, 
                                uint32_t AddrMode, uint32_t AddrSize, 
                                uint32_t DataMode)
{
    QSPI_CommandTypeDef Cmdhandler;
    
    Cmdhandler.Instruction = Inst;                 	
    Cmdhandler.Address = Addr;                            
    Cmdhandler.DummyCycles = dCycles;                   
    Cmdhandler.InstructionMode = InstMode;				
    Cmdhandler.AddressMode = AddrMode;   					
    Cmdhandler.AddressSize = AddrSize;   				
    Cmdhandler.DataMode = DataMode;             				
    Cmdhandler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;       	
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 
    Cmdhandler.DdrMode = QSPI_DDR_MODE_DISABLE;           	
    Cmdhandler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    if(HAL_QSPI_Command(&QSPI_Handler, &Cmdhandler, 0xffff) == HAL_OK)
    {
        return (QSPI_OK);
    }

    return (QSPI_ERR);
}

static uint8_t QSPI_Transmit(uint8_t *buff, uint32_t size)
{
    QSPI_Handler.Instance->DLR = size - 1;
    if(HAL_QSPI_Transmit(&QSPI_Handler, buff, 5000) == HAL_OK)
    {
        return (QSPI_OK);
    }
    return (QSPI_ERR);
}

static uint8_t QSPI_Receive(uint8_t *buff, uint32_t size)
{
    QSPI_Handler.Instance->DLR = size - 1;
    if(HAL_QSPI_Receive(&QSPI_Handler, buff, 5000) == HAL_OK)
    {
        return (QSPI_OK);
    }
    return (QSPI_ERR);
}

/********************** W25Qxxx control ********************************/
static uint8_t W25Q_SoftReset(uint8_t itface)
{
	if(itface == SerialSPI)
	{
		/* send reset enable cmd */
		QSPI_SendCommand(W25Q_RESET_ENABLE, 0, 
	                   		0, QSPI_INSTRUCTION_1_LINE, 
	                    	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
	                    	QSPI_DATA_NONE);
		/* delay */
	    TimDelayMs(1);
	    /* send reset enable */
	    QSPI_SendCommand(W25Q_RESET, 0, 
	                   		0, QSPI_INSTRUCTION_1_LINE, 
	                    	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
	                    	QSPI_DATA_NONE);

  	}
  	else if(itface == SerialQUADSPI)
  	{
		/* send reset enable cmd */
		QSPI_SendCommand(W25Q_RESET_ENABLE, 0, 
	                   		0, QSPI_INSTRUCTION_4_LINES, 
	                    	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
	                    	QSPI_DATA_NONE);
		/* delay */
	    TimDelayMs(1);
	    /* send reset enable */
	    QSPI_SendCommand(W25Q_RESET, 0, 
	                   		0, QSPI_INSTRUCTION_4_LINES, 
	                    	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
	                    	QSPI_DATA_NONE);

  	}
  	else
  	{
  	    printf("W25Q SoftRead, unsupport serial\r\n");
  		return (1);
  	}

  	return (0);
}

static bool W25Q_WriteEnable(uint8_t itface)
{
	uint8_t ret = QSPI_ERR, timeout = 20, tmp = 0;

	/* QSPI send command */
	if(itface == SerialSPI)
	{
		ret = QSPI_SendCommand(W25Q_WRITE_ENABLE, 0, 
                            	0, QSPI_INSTRUCTION_1_LINE, 
                             	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                              	QSPI_DATA_NONE);
        //TimDelayUs(20);
        ret = QSPI_SendCommand(W25Q_READ_SR1, 0, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_1_LINE);
	}
	else if(itface == SerialQUADSPI)
	{
    	ret = QSPI_SendCommand(W25Q_WRITE_ENABLE, 0, 
                           		0, QSPI_INSTRUCTION_4_LINES, 
                             	QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                             	QSPI_DATA_NONE);
        //TimDelayUs(20);
        ret = QSPI_SendCommand(W25Q_READ_SR1, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_4_LINES);
	}

	if(ret == QSPI_OK)
	{
		while(timeout--)
		{
			TimDelayUs(10);
			ret = QSPI_Receive(&tmp, 1);
			//printf("SR1 = %02x\r\n", tmp);
			if((tmp&0x02) == 0x02) {
				return (TRUE);
			}
		}
	}

	return (FALSE);
}

static uint8_t W25Q_WriteDisable(uint8_t itface)
{
    uint8_t ret = QSPI_ERR;
    
    /* QSPI send command */
    if(itface == SerialSPI)
    {
        ret = QSPI_SendCommand(W25Q_WRITE_DISABLE, 0, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(itface == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_WRITE_DISABLE, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_4_LINES);
    }

	if(ret != QSPI_OK)
	{
	    printf("W25Q disable write error!r\\n");
	    return (1);
	}

	return (0);
}

static uint8_t W25Q_ReadSR(uint32_t SRxRCmd, uint8_t itface)
{
    uint8_t ret = QSPI_ERR, Recv = 0;

    /* QSPI send command */
    if(itface == SerialSPI)
    {
        ret = QSPI_SendCommand(SRxRCmd, 0, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(itface == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(SRxRCmd, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_4_LINES);
    }

    /* send command error */ 
    if(ret != QSPI_OK) {
    	return (0);
	}

	ret = QSPI_Receive(&Recv, 1);
    if(ret != QSPI_OK) {
        return (0);
    }

    return (Recv);
}

static uint8_t W25Q_WriteSR(uint32_t SRxWCmd, uint8_t Val, uint8_t itface)
{
    uint8_t ret = QSPI_ERR;

    /* QSPI send command */
    if(itface == SerialSPI)
    {
        ret = QSPI_SendCommand(SRxWCmd, 0, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(itface == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(SRxWCmd, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_4_LINES);
    }

    /* send command error */ 
    if(ret != QSPI_OK) {
    	return (1);
	}

	ret = QSPI_Transmit(&Val, 1);
    if(ret != QSPI_OK) {
        return (2);
    }
    
    return (0);
}


static uint8_t W25Q_SetAddressMode(uint32_t AddrModeCmd)
{
    uint8_t ret = QSPI_ERR;
    
    /* QSPI send command */
    if(QFLSerial == SerialSPI)
    {
      	ret = QSPI_SendCommand(AddrModeCmd, 0, 
                        	    0, QSPI_INSTRUCTION_1_LINE, 
                      		    QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                       		    QSPI_DATA_NONE);
    }
    else if(QFLSerial == SerialQUADSPI)
    {
      	ret = QSPI_SendCommand(AddrModeCmd, 0, 
                          	    0, QSPI_INSTRUCTION_4_LINES, 
                         	    QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                        	    QSPI_DATA_NONE);
    }

    if(ret != QSPI_OK) {
        printf("W25Q set address mode error\r\n");
        return (1);
    }

    return (0);
}

/*
*   QSPI Only
*   dummy[bit5:4] 
*           0 -> 2dummy 40mhz
*           1 -> 4dummy 80mhz
*           2 -> 6dummy 104mhz
*           3 -> 8dummy 104mhz
*   wrap[bit1:0] 
*           0 -> 8byte
*           1 -> 16byte
*           2 -> 32byte
*           3 -> 64byte
*/
static uint8_t W25Q_SetReadParameter(uint8_t dummy, uint8_t wrap)
{
    uint8_t tmp = 0, ret = QSPI_ERR;
    
    /* QSPI send command */
    if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_READ_PARA, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_4_LINES);
    }

    /* send command error */ 
    if(ret != QSPI_OK)
    {
        printf("w25qxx Set Read Parameter error_1!\r\n");
        return (1);
    }

    tmp = wrap | dummy << 4;
    ret = QSPI_Transmit(&tmp, 1);
    if(ret != QSPI_OK)
    {
        printf("w25qxx Set Read Parameter error_2!\r\n");
        return (2);   
    }

    return (0);
}

static uint16_t W25Q_ReadDeviceID(void)
{
    uint8_t Recv[2];
    uint8_t ret = QSPI_ERR;

    /* QSPI send command */
#if 1	
    if(QFLSerial == SerialSPI)
    {
        ret = QSPI_SendCommand(W25Q_READID, 0, 
                                4, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_READID, 0, 
                                4, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_4_LINES);
    }	
#else
	ret = QSPI_SendCommand(W25Q_READID, 0, 
                       		0, QSPI_INSTRUCTION_1_LINE, 
                        	QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, 
                        	QSPI_DATA_1_LINE);
#endif

    /* send command error */ 
    if(ret != QSPI_OK) {
        //printf("w25qxx, get device ID error_1\r\n");
        return 0;
    }

    ret = QSPI_Receive(Recv, 2);
    if(ret != QSPI_OK) {
        //printf("w25qxx, get device ID error_2\r\n");
        return 0;
    }

    return (uint16_t)((uint16_t)(Recv[0] << 8) | Recv[1]);
}

static uint8_t W25Q_EnterQuadMode(void)
{
    uint8_t ret = 0, tmp = 0;

    /* try to read in SPI mode */
	tmp = W25Q_ReadSR(W25Q_READ_SR2, SerialSPI);
	if((tmp != 0xFF) && (tmp & 0x02)) {/* QE setbit but not in QPI mode */
	    QFLSerial = SerialSPI;
	    //printf("QE setbit but not in QPI mode\r\n");
		goto QPI_EN;
	}

    /* try to read in QSPI mode*/
	tmp = W25Q_ReadSR(W25Q_READ_SR2, SerialQUADSPI);
	if((tmp != 0xFF) && (tmp & 0x02)) {/* QE setbit and current in QPI mode */
	    QFLSerial = SerialQUADSPI;
	    //printf("QE setbit and current in QPI mode\r\n");
	    return (0);
	}

  	/* write enable first */
  	if(W25Q_WriteEnable(SerialSPI) != TRUE) {
        QFLSerial = SerialSPI;
        //printf("write diable\r\n");
        return (1);
  	}
  	
  	/* set QE bit */
 	tmp |= 0x02;
 	ret = W25Q_WriteSR(W25Q_WRITE_SR2, tmp, SerialSPI);
  	if(ret != 0) {
     	QFLSerial = SerialSPI;
      	return (2);
 	}    

QPI_EN:    
	/* send QSPI Enable cmd */
    ret = QSPI_SendCommand(W25Q_QSPI_ENABLE, 0, 
                            0, QSPI_INSTRUCTION_1_LINE, 
                            QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                            QSPI_DATA_NONE);
	/* delay */
	TimDelayUs(20);
	
    if(ret != QSPI_OK) {
        QFLSerial = SerialSPI;
        return (3);
    }
    
 	QFLSerial = SerialQUADSPI;
 	//printf("Serial mode QUADSPI\r\n");
    return (0);
}

static uint8_t W25Q_ExitQuadMode(void)
{
    uint8_t ret = QSPI_OK;

    if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_QSPI_DISABLE, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_24_BITS, 
                                QSPI_DATA_NONE);
    }

    if(ret != QSPI_OK) {
        printf("w25qxx disable QSPI error!\r\n");
        return (1);
    }

    QFLSerial = SerialSPI;
    return (0);
}

/* delay 1ms one tune 
*   @para
*       timeout : >0
*/
static bool W25Q_ChkFinished(uint32_t timeout)
{
    uint8_t tmp = 0, ret = QSPI_ERR;
    
    /* QSPI send command */
    if(QFLSerial == SerialSPI)
    {
        ret = QSPI_SendCommand(W25Q_READ_SR1, 0, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_READ_SR1, 0, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, 
                                QSPI_DATA_4_LINES);
    }

    if(ret != QSPI_OK) {
    	return (FALSE);
	}

    while(timeout--)
    {
        /* busy flag in SR1:bit0 */
     	QSPI_Receive(&tmp, 1);
     	if((tmp & 0x01) == 0) {
     	    return TRUE;
     	}     	
     	/* delay */
     	TimDelayMs(1);
    }

    return (FALSE);
}


/* erase time > 150ms */
static uint8_t W25Q_EraseSector(uint32_t addr)
{
    uint8_t ret = QSPI_ERR;

    if(W25Q_WriteEnable(QFLSerial) == FALSE) {
    	return (1);
    }
        
    /* send erase command */
    if(QFLSerial == SerialSPI)
    {
        ret = QSPI_SendCommand(W25Q_SECTOR_ERASE, addr, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_32_BITS, 
                                QSPI_DATA_NONE);
    }
    else if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_SECTOR_ERASE, addr, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_32_BITS, 
                                QSPI_DATA_NONE);
    }

    if(ret != QSPI_OK) {
        return (2);
    }
    //waite > 150ms
    if(W25Q_ChkFinished(200) == FALSE) {
        return (3);
    }

    return (0);
}

/*
*   Page Program, wSize <= 256. Just program in one page(0~65535).
*   QSPI Only
*/
static uint8_t W25Q_PageProgram(uint8_t *buff, uint32_t addr, uint16_t size)
{
    uint8_t ret = QSPI_ERR;

    if(W25Q_WriteEnable(QFLSerial) == FALSE) {
    	return (1);
    }
        
    /* send page program command */
    if(QFLSerial == SerialSPI)
    {
        ret = QSPI_SendCommand(W25Q_PAGE_PROGRAM, addr, 
                                0, QSPI_INSTRUCTION_1_LINE, 
                                QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_32_BITS, 
                                QSPI_DATA_1_LINE);
    }
    else if(QFLSerial == SerialQUADSPI)
    {
        ret = QSPI_SendCommand(W25Q_PAGE_PROGRAM, addr, 
                                0, QSPI_INSTRUCTION_4_LINES, 
                                QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_32_BITS, 
                                QSPI_DATA_4_LINES);
    }
    
	/* send page program command error */
	if(ret != QSPI_OK) {
		return (2);
    }
    
	/* QSPI send data */
	ret = QSPI_Transmit(buff, size);
	if(ret != QSPI_OK) {
	    return (3);
		
	}

	if(W25Q_ChkFinished(20) == FALSE) {
	    return (4);
    }

	return (0);
}

/* QSPI Only */
static uint8_t W25Q_Read(uint8_t *buff, uint32_t addr, uint16_t size)
{
    uint8_t ret = QSPI_ERR;

	/* send read command */
	if(QFLSerial == SerialSPI)
	{
	    ret = QSPI_SendCommand(W25Q_FAST_READ, addr, 
                             	8, QSPI_INSTRUCTION_1_LINE, 
                             	QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_32_BITS, 
                               	QSPI_DATA_1_LINE);
	}
	else if(QFLSerial == SerialQUADSPI)
	{
        ret = QSPI_SendCommand(W25Q_FAST_READ, addr, 
                             	8, QSPI_INSTRUCTION_4_LINES, 
                             	QSPI_ADDRESS_4_LINES, QSPI_ADDRESS_32_BITS, 
                               	QSPI_DATA_4_LINES);
    }
    
	/* send command error */ 
 	if(ret != QSPI_OK) {
		return (1);
    }
    
	/* QSPI receive data */
	ret = QSPI_Receive(buff, size);
	if(ret != QSPI_OK) {
		return (2);
    }

    return (0);
}



