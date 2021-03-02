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
#ifndef _PM_H_
#define _PM_H_

#include "comdef.h"
#include <stdbool.h>

#define __pm_item_register(name, idle, suspend, resume)\
    USED ANONY_TYPE(const pm_item_t,__pm_item_##suspend)\
    SECTION("pm.item.1") = {name, idle, suspend, resume}

typedef struct {
    /**
     * @brief   设备名称
     */    
    const char *name;                      
    /**
     * @brief   当前设备工作状态(只有所有的pm_item_t都允许休眠,系统才能休眠)
     * @retval  true - 空闲状态允许休眠,false - 正常模式,不允许休眠
     */
    bool   (*idle)(void);
    /**
     * @brief      挂起设备
     * @param[out] next_wakup_time - 下次唤醒时间
     * @retval     唤醒时间(单位:ms, 0 - 表示由于系统决定)
     */    
    void  (*suspend)(unsigned int *next_wakup_time);
    /**
     * @brief   恢复
     * @retval  none
     */    
    void  (*resume)(void);
}pm_item_t;

/*低功耗适配器 ---------------------------------------------------------------*/
typedef struct {
    /**
     * @brief    系统最大休眠时长(ms)
     */  
    unsigned int max_sleep_time;
    /**
     * @brief     进入休眠状态
     * @param[in] time - 期待休眠时长(ms)
     * @retval    实际休眠时长
     */     
    void (*goto_sleep)(unsigned int time);
}pm_adapter_t;

/**
 * @brief     功耗管理项注册
 * @param[in] name    - 项目名称
 * @param[in] idle    - 指示设备是否空闲
 * @param[in] suspend - 系统休眠通知
 * @param[in] resume  - 唤醒通知
 */ 
#define pm_dev_register(name, idle, suspend, resume)\
__pm_item_register(name, idle, suspend, resume)


void pm_init(const pm_adapter_t *adt);

void pm_process(void);


#endif