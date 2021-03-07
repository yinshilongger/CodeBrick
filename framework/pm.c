/******************************************************************************
 * @brief    功耗管理(power manager)
 *
 * Copyright (c) 2021, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2021-03-02     Morro        Initial version. 
 ******************************************************************************/
#include "pm.h"
#include <stddef.h>

/**
 * @brief   pm监视器
 */
typedef struct {
    const pm_adapter_t *adt;  
    bool  enable;
}pm_watch_t;

static pm_watch_t pm_watch;

static const pm_item_t pm_tbl_start SECTION("pm.item.0");

static const pm_item_t pm_tbl_end SECTION("pm.item.2");

/*
 * @brief  系统空闲检测
 */
static bool system_is_idle(void)
{
    const pm_item_t *it;
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->idle != NULL && !it->idle())
            return false;
    }
    return true;
}

/*
 * @brief  系统进行休眠处理
 */
static void system_goto_sleep(void)
{
    const pm_item_t   *it;
    const pm_adapter_t *adt;
    unsigned int sleep_time;
    unsigned int tmp;
    
    adt = pm_watch.adt;
    
    sleep_time = adt->max_sleep_time;
    
    //休眠处理
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->sleep_notify == NULL)
            continue;
        tmp = it->sleep_notify();          //休眠请求,并得到设备期待下次唤醒时间
        if (tmp && tmp < sleep_time)       //计算出所有设备中的最小允许休眠时间
            sleep_time = tmp;
    }
    
    adt->goto_sleep(sleep_time);
    
    //唤醒处理
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->wakeup_notify == NULL)
            continue;
        it->wakeup_notify();
    }
}

/**
 * @brief   初始化功耗管理器
 * @retval  adt - 适配器接口
 */
void pm_init(const pm_adapter_t *adt)
{
    pm_watch.adt = adt;
}

/**
 * @brief   启动功耗管理
 * @retval  none
 */
void pm_enable(void)
{
    pm_watch.enable = true;
}

/**
 * @brief   禁用功耗管理
 * @retval  none
 */
void pm_disable(void)
{
    pm_watch.enable = false;
}

/**
 * @brief   功耗管理
 * @retval  none
 */
void pm_process(void)
{
    if (!pm_watch.enable || !system_is_idle())
        return;
    system_goto_sleep();
}
