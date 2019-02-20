/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
* Filename : app_cfg.h
*********************************************************************************************************
*/

#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           TASK PRIORITIES
*********************************************************************************************************
*/

#define OS_TASK_TMR_PRIO        (OS_LOWEST_PRIO - 2u)  /* Only required for uC/OS-II                           */

#define STARTUP_TASK_PRIO       (3u)

#define APP_TASK_PRIO           (4u)
#define KPD_TASK_PRIO           (5u)
#define TP_TASK_PRIO            (6u)

#define LED_TASK_PRIO           (OS_TASK_TMR_PRIO - 1u)


/*
*********************************************************************************************************
*                                          TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define STARTUP_TASK_STK_SIZE       (128u)

#define APP_TASK_STK_SIZE           (1024u)
#define KPD_TASK_STK_SIZE           (128u)
#define TP_TASK_STK_SIZE            (128u)

#define LED_TASK_STK_SIZE           (64u)
/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#if 0
#define  APP_TRACE_LEVEL                   TRACE_LEVEL_OFF
#define  APP_TRACE                         printf

#define  APP_TRACE_INFO(x)    ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)     ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)
#endif

/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include. */

