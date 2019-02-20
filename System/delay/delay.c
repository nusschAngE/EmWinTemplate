
#include "stm32h7xx.h"
#include "timer.h"
#include "delay.h"
#include "ucos_ii.h"


#define DelayTIMCLK         (200 * 1000 * 1000)//200MHZ

#define DelayTIM_PSC_US		((DelayTIMCLK/1000/1000) - 1)
#define DelayTIM_ARR_1US	(1)

#define DelayTIM_PSC_MS		((DelayTIMCLK/1000) - 1)
#define DelayTIM_ARR_1MS	(1000)

static TIM_HandleTypeDef TIM2_Handle;

uint8_t DelayModule_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
	TIM2_Handle.Instance = TIM2;
	TIM2_Handle.Init.Prescaler = DelayTIM_PSC_US;
	TIM2_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM2_Handle.Init.Period = 1;
	TIM2_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM2_Handle.Init.RepetitionCounter = 0x00;
	TIM2_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&TIM2_Handle);
	
    return (0);
}

void TimDelayUs(uint32_t us)
{
	volatile uint32_t count = us;

    if(count <= 3)
    {
        DelayUs(count);
    }
    else
    {
        count -= 2;
    	__HAL_TIM_CLEAR_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE);
    	__HAL_TIM_SET_AUTORELOAD(&TIM2_Handle, DelayTIM_ARR_1US);
    	HAL_TIM_Base_Start(&TIM2_Handle);
    	
    	while(count--)
    	{
    		while(__HAL_TIM_GET_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE) == RESET)
    		{
    		}
    		__HAL_TIM_CLEAR_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE);
    	}

    	HAL_TIM_Base_Stop(&TIM2_Handle);
	}
}

void TimDelayMs(uint32_t ms)
{
	volatile uint32_t count = ms;

	__HAL_TIM_CLEAR_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE);
	__HAL_TIM_SET_AUTORELOAD(&TIM2_Handle, DelayTIM_ARR_1MS);
	HAL_TIM_Base_Start(&TIM2_Handle);
	
	while(count--)
	{
		while(__HAL_TIM_GET_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE) == RESET)
		{
		}
		__HAL_TIM_CLEAR_FLAG(&TIM2_Handle, TIM_FLAG_UPDATE);
	}    

	HAL_TIM_Base_Stop(&TIM2_Handle);
}

void SysTickDelay(uint32_t ticks)
{

}

/*
*   ~1us in 400mhz
*   @para : us < 0xFFFF
*/
void DelayUs(uint16_t us)
{
    volatile uint32_t Count = 80 * us;

    while(Count--)
    {
    }
}

/*
*   ~1ms in 400mhz
*   @para : 
*/
void DelayMs(uint16_t ms)
{
    volatile uint64_t Count = 80000 * ms;

    while(Count--)
    {
    }
}

void OSDelay(uint32_t ticks)
{
    OSTimeDly(ticks);
}

/*
*   @param : ticks 1ms
*   @ret : TRUE - OK
*          FALSE - Timeout
*/
bool WaitCondition(uint32_t ticks, bool(*condition)(void))
{
    volatile uint64_t Count = 65000 * ticks;

    while(Count--)
    {
        if(condition()) {
            return (TRUE);
        }
    }

    return (FALSE);
}

bool OSWaitCondition(uint32_t ticks, bool(*condition)(void))
{
    if(!condition) {return (FALSE);}

    while(ticks--)
    {
        if(condition() == TRUE) {
            return (TRUE);
        }

        OSTimeDly(1);
    }

    return (FALSE);
}

