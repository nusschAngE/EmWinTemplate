
#ifndef _STM_FLASH_H
#define _STM_FLASH_H

#include "public.h"







uint8_t STM_Flash_Write(uint32_t addr, uint32_t *buff, uint32_t Nbr2Write);
uint8_t STM_Flash_Read(uint32_t addr, uint32_t *buff, uint32_t NbrToRead);




#endif









