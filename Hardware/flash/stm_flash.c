
#include "stm32h7xx.h"
#include "stm_flash.h"

#define STM_FLASH_BASE      ((uint32_t)0x08000000) 	//STM32 Flash base
#define FLASH_WAITETIME     50000          //FLASH Timeout

//STM32H7 FLASH sector
#define FLASH_SECTOR_NONE           (8)
#define FLASH_BANK_NONE             (0)
//BANK1
#define ADDR_FLASH_SECTOR0_BANK1    ((uint32_t)0x08000000) //128 Kbytes  
#define ADDR_FLASH_SECTOR1_BANK1    ((uint32_t)0x08020000) //128 Kbytes  
#define ADDR_FLASH_SECTOR2_BANK1    ((uint32_t)0x08040000) //128 Kbytes  
#define ADDR_FLASH_SECTOR3_BANK1    ((uint32_t)0x08060000) //128 Kbytes  
#define ADDR_FLASH_SECTOR4_BANK1    ((uint32_t)0x08080000) //128 Kbytes  
#define ADDR_FLASH_SECTOR5_BANK1    ((uint32_t)0x080A0000) //128 Kbytes  
#define ADDR_FLASH_SECTOR6_BANK1    ((uint32_t)0x080C0000) //128 Kbytes  
#define ADDR_FLASH_SECTOR7_BANK1    ((uint32_t)0x080E0000) //128 Kbytes  
#define ADDR_FLASH_BANK1_END        ((uint32_t)0x080FFFFF)

//BANK2
#define ADDR_FLASH_SECTOR0_BANK2    ((uint32_t)0x08100000) //128 Kbytes  
#define ADDR_FLASH_SECTOR1_BANK2    ((uint32_t)0x08120000) //128 Kbytes  
#define ADDR_FLASH_SECTOR2_BANK2    ((uint32_t)0x08140000) //128 Kbytes  
#define ADDR_FLASH_SECTOR3_BANK2    ((uint32_t)0x08160000) //128 Kbytes  
#define ADDR_FLASH_SECTOR4_BANK2    ((uint32_t)0x08180000) //128 Kbytes  
#define ADDR_FLASH_SECTOR5_BANK2    ((uint32_t)0x081A0000) //128 Kbytes  
#define ADDR_FLASH_SECTOR6_BANK2    ((uint32_t)0x081C0000) //128 Kbytes  
#define ADDR_FLASH_SECTOR7_BANK2    ((uint32_t)0x081E0000) //128 Kbytes 
#define ADDR_FLASH_BANK2_END        ((uint32_t)0x081FFFFF)

#define IS_STMFLASH_BANK1(addr)     ((addr >= ADDR_FLASH_SECTOR0_BANK1) && (addr <= ADDR_FLASH_BANK1_END))
#define IS_STMFLASH_BANK2(addr)     ((addr >= ADDR_FLASH_SECTOR0_BANK2) && (addr <= ADDR_FLASH_BANK2_END))

uint32_t STM_Flash_GetSector(uint32_t addr)
{
    if(IS_STMFLASH_BANK1(addr))
    {
        if(addr < ADDR_FLASH_SECTOR1_BANK1) {return (FLASH_SECTOR_0);}
        else if(addr < ADDR_FLASH_SECTOR2_BANK1) {return (FLASH_SECTOR_1);}
        else if(addr < ADDR_FLASH_SECTOR3_BANK1) {return (FLASH_SECTOR_2);}
        else if(addr < ADDR_FLASH_SECTOR4_BANK1) {return (FLASH_SECTOR_3);}
        else if(addr < ADDR_FLASH_SECTOR5_BANK1) {return (FLASH_SECTOR_4);}
        else if(addr < ADDR_FLASH_SECTOR6_BANK1) {return (FLASH_SECTOR_5);}
        else if(addr < ADDR_FLASH_SECTOR7_BANK1) {return (FLASH_SECTOR_6);}
        else if(addr < ADDR_FLASH_BANK1_END) {return (FLASH_SECTOR_7);}
    }
    else if(IS_STMFLASH_BANK2(addr))
    {
        if(addr < ADDR_FLASH_SECTOR1_BANK2) {return (FLASH_SECTOR_0);}
        else if(addr < ADDR_FLASH_SECTOR2_BANK2) {return (FLASH_SECTOR_1);}
        else if(addr < ADDR_FLASH_SECTOR3_BANK2) {return (FLASH_SECTOR_2);}
        else if(addr < ADDR_FLASH_SECTOR4_BANK2) {return (FLASH_SECTOR_3);}
        else if(addr < ADDR_FLASH_SECTOR5_BANK2) {return (FLASH_SECTOR_4);}
        else if(addr < ADDR_FLASH_SECTOR6_BANK2) {return (FLASH_SECTOR_5);}
        else if(addr < ADDR_FLASH_SECTOR7_BANK2) {return (FLASH_SECTOR_6);}
        else if(addr < ADDR_FLASH_BANK2_END) {return (FLASH_SECTOR_7);}
    }

    return (FLASH_SECTOR_NONE);
}

uint32_t STM_Flash_GetBank(uint32_t addr)
{
    if(IS_STMFLASH_BANK1(addr)) {return (FLASH_BANK_1);}
    if(IS_STMFLASH_BANK2(addr)) {return (FLASH_BANK_2);}

    return (FLASH_BANK_NONE);
}

uint32_t STM_Flash_ReadWord(uint32_t addr)
{
    if(IS_STMFLASH_BANK1(addr) || IS_STMFLASH_BANK2(addr))
    {
        return (*(__IO uint32_t*)addr);
    }

    return (uint32_t)0;
}

uint8_t STM_Flash_Write(uint32_t addr, uint32_t *buff, uint32_t NbrToWrite)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t sectorError = 0;
    uint32_t writeAddr = 0, endAddr = 0, chkAddr = 0;
    uint32_t *writePtr = buff;

    if((STM_Flash_GetBank(addr) == 0) || (addr%4 != 0) || (NbrToWrite%8 != 0)) 
    {
        printf("error param!!\r\n");
        return (HAL_ERROR);
    }

    writeAddr = addr;
    chkAddr = addr;
    endAddr = addr + NbrToWrite*4;
    /* unlock the stm32 flash */
    HAL_FLASH_Unlock();
    /* check&erase */
    while(chkAddr < endAddr)
    {
        if(STM_Flash_ReadWord(chkAddr) != 0xFFFFFFFF)//need erase
        {
            FlashEraseInit.Banks = STM_Flash_GetBank(chkAddr);
            FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
            FlashEraseInit.Sector = STM_Flash_GetSector(chkAddr);
            FlashEraseInit.NbSectors = 1;
            FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;//2.7V~3.6V
            if(HAL_FLASHEx_Erase(&FlashEraseInit, &sectorError) != HAL_OK)
            {
                printf("flash erase[bank%d - sector%d] error!!\r\n", STM_Flash_GetBank(chkAddr), 
                                                                     STM_Flash_GetSector(chkAddr));
                return (HAL_ERROR); 
            }
            //wait operate done
            if(FLASH_WaitForLastOperation(FLASH_WAITETIME, STM_Flash_GetBank(chkAddr)) != HAL_OK)
            {
                printf("flash erase timeout!!\r\n");
                return (HAL_ERROR); 
            }
        }
        chkAddr += 4;//4 bytes
    }
    /* stm flash programs */
    while(writeAddr < endAddr)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, writeAddr, (uint64_t)writePtr) != HAL_OK)
        {
            printf("flash program error!!\r\n");
            return (HAL_ERROR);
        }
        writeAddr += 32;
        writePtr += 8;
    }
    /* write done, lock the stm32 flash */
    HAL_FLASH_Lock();
    
    return (HAL_OK);
}

uint8_t STM_Flash_Read(uint32_t addr, uint32_t *buff, uint32_t NbrToRead)
{
    uint32_t idx = 0;

    for(idx = 0; idx < NbrToRead; idx++)
    {
        buff[idx] = *(__IO uint32_t *)addr;
        addr += 4;
    }

    return (HAL_OK);
}


