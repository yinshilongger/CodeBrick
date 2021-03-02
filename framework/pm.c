/******************************************************************************
 * @brief    功耗管理(power manager)
 *
 * Copyright (c) 2021, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2021-01-17     Morro        Initial version. 
 ******************************************************************************/
#include "pm.h"
#include <stddef.h>

/**
 * @brief   pm控制器
 */ 
static const pm_adapter_t *pm_ctrl;

static const pm_item_t pm_tbl_start SECTION("init.item.0");

static const pm_item_t pm_tbl_end SECTION("init.item.2");

/*
 * @brief  系统空闲检测
 */
static bool system_is_idle(void)
{
    const pm_item_t *it;
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (!it->idle())
            return false;
    }
    return true;
}

/*
 * @brief  系统进行休眠处理
 */
static void system_goto_sleep(void)
{
    const pm_item_t *it;
    unsigned int tmp;
    unsigned int sleep_time = pm_ctrl->max_sleep_time;
    
    //休眠处理
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->suspend == NULL)
            continue;
        it->suspend(&tmp);                 //挂起设备,并得到设备期待下次唤醒时间
        if (tmp < sleep_time)              //计算出所有设备中的最小允许休眠时间
            sleep_time = tmp;
    }
    
    pm_ctrl->goto_sleep(sleep_time);
    
    //唤醒处理
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->resume == NULL)
            continue;
        it->resume();
    }
}

/**
 * @brief   初始化功耗管理器
 * @retval  adt - 适配器接口
 */
void pm_init(const pm_adapter_t *adt)
{
    pm_ctrl = adt;
}

/**
 * @brief   功耗管理
 * @retval  none
 */
void pm_process(void)
{
    if (!system_is_idle())
        return;
    system_goto_sleep();
}