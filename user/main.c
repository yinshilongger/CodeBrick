/******************************************************************************
 * @brief    
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 ******************************************************************************/
#include "config.h"
#include "module.h"
#include <stdio.h>

/* 
 * @brief        123
 * @return      none
 */
int main(void)
{    
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_ADDRESS);
    module_task_init();                         /*ģʼ*/
    while (1) {    
        module_task_process();                   /*ѯ*/
    }
}
