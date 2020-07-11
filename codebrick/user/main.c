/******************************************************************************
 * @brief    主程序入口
 *
 * Copyright (c) 2020, <master_roger@sina.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 ******************************************************************************/

#include "module.h"
#include <stdio.h>

/* 
 * @brief       主程序入口 
 * @return      none
 */
int main(void)
{
    module_task_init();                         /*模块初始化*/
    while (1) {    
        module_task_process();                   /*任务轮询*/
    }
}
