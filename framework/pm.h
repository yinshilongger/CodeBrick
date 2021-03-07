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
     * @note    如果填NULL,则表示允许系统休眠
     * @retval  true - 空闲状态允许休眠,false - 正常模式,不允许休眠
     */
    bool   (*idle)(void);
    /**
     * @brief      休眠通知
     * @retval     下次唤醒时间(单位:ms, 0 - 表示由于系统决定)
     */    
    unsigned int  (*sleep_notify)(void);
    /**
     * @brief     唤醒通知
     * @retval  none
     */    
    void  (*wakeup_notify)(void);
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
     * @note      休眠之后需要考虑两件事情,1个是需要定时起来给喂看门狗,否则会在休眠
     *            期间发送重启.另外一件事情是需要补偿休眠时间给系统滴答时钟,否则会
     *            造成时间不准。
     */     
    unsigned int (*goto_sleep)(unsigned int time);
}pm_adapter_t;

/**
 * @brief     功耗管理项注册
 * @param[in] name    - 项目名称
 * @param[in] idle    - 指示设备是否空闲,如果填NULL,则表示允许系统休眠
 * @param[in] sleep_notify   - 休眠通知,不需要则填NULL
 * @param[in] wakeup_notify  - 唤醒通知,不需要则填NULL
 */ 
#define pm_dev_register(name, idle, sleep_notify, wakeup_notify)\
__pm_item_register(name, idle, sleep_notify, wakeup_notify)


void pm_init(const pm_adapter_t *adt);

void pm_enable(void);

void pm_disable(void);

void pm_process(void);


#endif

