/******************************************************************************
 * @brief    ���Ĺ���(power manager)
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
 * @brief   pm������
 */
typedef struct {
    const pm_adapter_t *adt;  
    bool  enable;
}pm_watch_t;

static pm_watch_t pm_watch;

static const pm_item_t pm_tbl_start SECTION("pm.item.0");

static const pm_item_t pm_tbl_end SECTION("pm.item.2");

/*
 * @brief  ϵͳ���м��
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
 * @brief  ϵͳ�������ߴ���
 */
static void system_goto_sleep(void)
{
    const pm_item_t   *it;
    const pm_adapter_t *adt;
    unsigned int sleep_time;
    unsigned int tmp;
    
    adt = pm_watch.adt;
    
    sleep_time = adt->max_sleep_time;
    
    //���ߴ���
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->sleep_notify == NULL)
            continue;
        tmp = it->sleep_notify();          //��������,���õ��豸�ڴ��´λ���ʱ��
        if (tmp && tmp < sleep_time)       //����������豸�е���С��������ʱ��
            sleep_time = tmp;
    }
    
    adt->goto_sleep(sleep_time);
    
    //���Ѵ���
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->wakeup_notify == NULL)
            continue;
        it->wakeup_notify();
    }
}

/**
 * @brief   ��ʼ�����Ĺ�����
 * @retval  adt - �������ӿ�
 */
void pm_init(const pm_adapter_t *adt)
{
    pm_watch.adt = adt;
}

/**
 * @brief   �������Ĺ���
 * @retval  none
 */
void pm_enable(void)
{
    pm_watch.enable = true;
}

/**
 * @brief   ���ù��Ĺ���
 * @retval  none
 */
void pm_disable(void)
{
    pm_watch.enable = false;
}

/**
 * @brief   ���Ĺ���
 * @retval  none
 */
void pm_process(void)
{
    if (!pm_watch.enable || !system_is_idle())
        return;
    system_goto_sleep();
}
