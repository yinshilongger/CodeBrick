/******************************************************************************
 * @brief    平台相关初始化(无低功耗管理)
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ******************************************************************************/
#include "module.h"
#include "public.h"
#include "config.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "tty.h"

/*
 * @brief	   系统滴答中断
 * @param[in]   none
 * @return 	   none
 */
void SysTick_Handler(void)
{
    systick_increase(SYS_TICK_INTERVAL);
}

/*
 * @brief	   重定向printf
 */
int fputc(int c, FILE *f)
{       
    tty.write(&c, 1);
    while (tty.tx_isfull()) {}                           //防止丢LOG
    return c;
}

/*
 * @brief	   硬件驱动初始化
 * @param[in]   none
 * @return 	    none
 */
static void bsp_init(void)
{    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
    tty.init(115200);
    SystemCoreClockUpdate();
 	SysTick_Config(SystemCoreClock / (1000 / SYS_TICK_INTERVAL));   //配置系统时钟
	NVIC_SetPriority(SysTick_IRQn, 0);    
    wdog_conf(MAX_DOG_FEED_TIME);                       //初始化看门狗
}system_init("bsp", bsp_init); 

/*
 * @brief	   喂狗任务
 */
static void wdog_task(void)
{
    IWDG_ReloadCounter();
}task_register("dog", wdog_task, 1000);
