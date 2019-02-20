
#include "stm32h7xx.h"
#include "timer.h"


/*
*   TIM234567 IN APB1[100MHZ]
*   TIM234567 CORE CLOCK IS 200MHZ WITCH IS CONFIGRATED AS EQUAL TO HCLK
*/
static TIM_HandleTypeDef TIM3_Handle;
static TIM_HandleTypeDef TIM4_Handle;
static void (*Tim3TriggerCallback)(void);
static void (*Tim4TriggerCallback)(void);

/*****************************************
*   TIM3 FUNCTION
******************************************/

void TIM3_Init(TimInit_t *init)
{
	__HAL_RCC_TIM3_CLK_ENABLE();
	TIM3_Handle.Instance = TIM3;
	TIM3_Handle.Init.Prescaler = init->psc;
	TIM3_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM3_Handle.Init.Period = init->arr;
	TIM3_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM3_Handle.Init.RepetitionCounter = 0x00;
	TIM3_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&TIM3_Handle);  

    /* Enable IRQ */
    HAL_NVIC_SetPriority(TIM3_IRQn,2,2); 
    HAL_NVIC_EnableIRQ(TIM3_IRQn); 

    /* Callback */
    Tim3TriggerCallback = init->TriggerFunc;

    /* Start IT */
    HAL_TIM_Base_Start_IT(&TIM3_Handle);
}

void TIM3_DeInit(TimInit_t *init)
{
    __HAL_TIM_DISABLE(&TIM3_Handle);
    __HAL_RCC_TIM3_CLK_DISABLE();
    Tim3TriggerCallback = NULL;
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handle);
}

/*******************************************
*   TIM4 FUNCTION
********************************************/
void TIM4_Init(TimInit_t *init)
{
	__HAL_RCC_TIM4_CLK_ENABLE();
	TIM4_Handle.Instance = TIM4;
	TIM4_Handle.Init.Prescaler = init->psc;
	TIM4_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM4_Handle.Init.Period = init->arr;
	TIM4_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM4_Handle.Init.RepetitionCounter = 0x00;
	TIM4_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&TIM4_Handle);  

    /* Enable IRQ */
    HAL_NVIC_SetPriority(TIM4_IRQn,2,3); 
    HAL_NVIC_EnableIRQ(TIM4_IRQn); 

    /* Callback */
    Tim4TriggerCallback = init->TriggerFunc;

    /* Start IT */
    HAL_TIM_Base_Start_IT(&TIM4_Handle);
}

void TIM4_DeInit(TimInit_t *init)
{
    __HAL_TIM_DISABLE(&TIM4_Handle);
    __HAL_RCC_TIM4_CLK_DISABLE();
    Tim4TriggerCallback = NULL;
}

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM4_Handle);
}

/***********************************************
*   TIM TRIGGER CALLBACK
************************************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* TIM4 Trigger */
    if(htim == &TIM3_Handle)
    {
        if(Tim3TriggerCallback) {
            Tim3TriggerCallback();
        }
    }

    /* TIM4 Trigger */
    if(htim == &TIM4_Handle)
    {
        if(Tim4TriggerCallback) {
            Tim4TriggerCallback();
        }
    }
}

