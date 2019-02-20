

#include "stm32h7xx.h"
#include "system_conf.h"
#include "public.h"

SystemClock_Struct SystemClock;

/*
*   system clock configuration
*   must be called firstly in main()
*/
void System_ClockConfig(void)
{
  	RCC_OscInitTypeDef RCC_OscInitStruct;
  	RCC_ClkInitTypeDef RCC_ClkInitStruct;
  	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Supply configuration update enable 
    */
  	MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

    /**Configure the main internal regulator output voltage 
    */
  	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  	while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
  	{
    
  	}
    /**Macro to configure the PLL clock source 
    */
  	__HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);//25mhz

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
  	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  	RCC_OscInitStruct.HSICalibrationValue = 16;
  	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  	RCC_OscInitStruct.PLL.PLLM = 5; //5mhz 
  	RCC_OscInitStruct.PLL.PLLN = 160;//800mhz
  	RCC_OscInitStruct.PLL.PLLP = 2;//400mhz -> system clock
  	RCC_OscInitStruct.PLL.PLLQ = 4;//200mhz
  	RCC_OscInitStruct.PLL.PLLR = 4;
  	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
  	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  	{
    	while(1);
  	}

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              	|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              	|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;//400MHz
  	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;//200mhz
  	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;//100mhz
  	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  	{
    	while(1);
  	}

  	/* PLL2 Config Struct init, input source : HSE->25mhz */                                            
  	PeriphClkInitStruct.PLL2.PLL2M = 25;   //1mhz
  	PeriphClkInitStruct.PLL2.PLL2N = 400;  //400mhz
  	PeriphClkInitStruct.PLL2.PLL2P = 2;   //240mhz
  	PeriphClkInitStruct.PLL2.PLL2Q = 2;
  	PeriphClkInitStruct.PLL2.PLL2R = 2;
  	PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  	PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  	PeriphClkInitStruct.PLL2.PLL2FRACN = 0;

    /* PLL3 Config Struct init, input source : HSE->25mhz */
    PeriphClkInitStruct.PLL3.PLL3M = 5;   //5mhz
  	PeriphClkInitStruct.PLL3.PLL3N = 96; //480mhz
  	PeriphClkInitStruct.PLL3.PLL3P = 6;   
  	PeriphClkInitStruct.PLL3.PLL3Q = 6;
  	PeriphClkInitStruct.PLL3.PLL3R = 6;
  	PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
  	PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  	PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

    /* Peripheral Clock Configuration Selection */
  	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART16//uart1&6
  	                                            | RCC_PERIPHCLK_USART234578
  	                                            | RCC_PERIPHCLK_SDMMC
  	                                            | RCC_PERIPHCLK_USB
  	                                            | RCC_PERIPHCLK_SAI1
  	                                            | RCC_PERIPHCLK_QSPI //200mhz
  	                                            | RCC_PERIPHCLK_FMC//200mhz
  	                                            | RCC_PERIPHCLK_SPI2//200MHZ
  	                                            | RCC_PERIPHCLK_TIM;//200mhz
  	/* Peripheral Clock Source Selection */                                            
  	PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL;//PLL1Q, PLL2R
  	PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL3;//PLL2Q, PLL3Q
  	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3;//PLL2Q, PLL3Q
  	PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL;//PLL1Q, PLL2R
  	PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;//PLL1Q, PLL2R
  	PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_ACTIVATED;
  	PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  	PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  	PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;//PLL1Q, PLL2P, PLL3P
  	/* Peripheral Clock Configuration */
  	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  	{
    	while(1);
  	}

  	__HAL_RCC_CSI_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();  
	HAL_EnableCompensationCell();
#if 0
    /* Get CPU clock frequency
    */
    SystemCoreClockUpdate();
    
    /**Configure the Systick interrupt time 
    */
  	HAL_SYSTICK_Config(SystemCoreClock/1000);
    /**Configure the Systick 
    */
  	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
  	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);    
#endif
}

void System_SysTickInit(uint32_t freq)
{
/* Get CPU clock frequency
    */
    SystemCoreClockUpdate();
    
    /**Configure the Systick interrupt time 
    */
  	HAL_SYSTICK_Config(SystemCoreClock/freq);
    /**Configure the Systick 
    */
  	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
  	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0); 	
}

/*
*   system pll2 configuration
*   it will disable PLL2 firstly, must be carefal
*/
void System_PLL2Config(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r)
{
    /* Check that PLL2 OSC clock source is already set */
    if(__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_NONE)
    {
        return;
    }
    
    /* Disable  PLL2. */
    __HAL_RCC_PLL2_DISABLE();
    
    /* Configure PLL2 multiplication and division factors. */
    __HAL_RCC_PLL2_CONFIG(m, n, p, q, r);
    
    /* Select PLL2 input reference frequency range: VCI */  
    __HAL_RCC_PLL2_VCIRANGE(RCC_PLL2VCIRANGE_0);
    
    /* Select PLL2 output frequency range : VCO */
    __HAL_RCC_PLL2_VCORANGE(RCC_PLL2VCOWIDE);
    
    /* Enable  PLL2. */
    __HAL_RCC_PLL2_ENABLE();
}

/*
*   system pll3 configuration
*   it will disable PLL3 firstly, must be carefal
*/
void System_PLL3Config(uint32_t m, uint32_t n, uint32_t p, uint32_t q, uint32_t r)
{
    /* Check that PLL3 OSC clock source is already set */
    if(__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_NONE)
    {
        return;
    }
    
    /* Disable  PLL3. */
    __HAL_RCC_PLL3_DISABLE();
    
    /* Configure PLL3 multiplication and division factors. */
    __HAL_RCC_PLL3_CONFIG(m, n, p, q, r);
    
    /* Select PLL3 input reference frequency range: VCI */  
    __HAL_RCC_PLL3_VCIRANGE(RCC_PLL3VCIRANGE_0);
    
    /* Select PLL3 output frequency range : VCO */
    __HAL_RCC_PLL3_VCORANGE(RCC_PLL3VCOWIDE);
    
    /* Enable  PLL3. */
    __HAL_RCC_PLL3_ENABLE();
}

/* DCache & ICache configuration */
void System_SCBCacheConfig(void)
{
    SCB_EnableDCache();
    SCB_EnableICache();

    SCB->CACR |= 1<<2;   //force D-Cache Writethrough
}

/* MPU configuration */
void System_MPUConfig(void)
{
	MPU_Region_InitTypeDef MPU_Initure;

	HAL_MPU_Disable();					

#if 1
	/* SDRAM : FMC SDRAM Bank1 */
	MPU_Initure.Enable = MPU_REGION_ENABLE;	
	MPU_Initure.Number = MPU_REGION_NUMBER0;	
	MPU_Initure.BaseAddress = (uint32_t)0xC0000000;
	MPU_Initure.Size = MPU_REGION_SIZE_32MB;		
	MPU_Initure.SubRegionDisable = 0x00;
	MPU_Initure.TypeExtField = MPU_TEX_LEVEL1;
	MPU_Initure.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_Initure.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_Initure.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_Initure.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_Initure);
#endif
	
	/* LCD : FMC SRAM Bank1 */
	MPU_Initure.Enable = MPU_REGION_ENABLE;	
	MPU_Initure.Number = MPU_REGION_NUMBER1;	
	MPU_Initure.BaseAddress = (uint32_t)0x60000000;
	MPU_Initure.Size = MPU_REGION_SIZE_32MB;		
	MPU_Initure.SubRegionDisable = 0x00;
	MPU_Initure.TypeExtField = MPU_TEX_LEVEL1;
	MPU_Initure.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_Initure.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_Initure.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_Initure.IsBufferable = MPU_ACCESS_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_Initure);
	
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT); 
}

#if 0
void HAL_MspInit(void)
{
    system_ClockConfig();
    system_SCBCacheConfig();
    system_MPUConfig();
}
#endif


