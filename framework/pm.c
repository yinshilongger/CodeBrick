/******************************************************************************
 * @brief    ���Ĺ���(power manager)
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
 * @brief   pm������
 */ 
static const pm_adapter_t *pm_ctrl;

static const pm_item_t pm_tbl_start SECTION("init.item.0");

static const pm_item_t pm_tbl_end SECTION("init.item.2");

/*
 * @brief  ϵͳ���м��
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
 * @brief  ϵͳ�������ߴ���
 */
static void system_goto_sleep(void)
{
    const pm_item_t *it;
    unsigned int tmp;
    unsigned int sleep_time = pm_ctrl->max_sleep_time;
    
    //���ߴ���
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->suspend == NULL)
            continue;
        it->suspend(&tmp);                 //�����豸,���õ��豸�ڴ��´λ���ʱ��
        if (tmp < sleep_time)              //����������豸�е���С��������ʱ��
            sleep_time = tmp;
    }
    
    pm_ctrl->goto_sleep(sleep_time);
    
    //���Ѵ���
    for (it = &pm_tbl_start + 1; it < &pm_tbl_end; it++) {
        if (it->resume == NULL)
            continue;
        it->resume();
    }
}

/**
 * @brief   ��ʼ�����Ĺ�����
 * @retval  adt - �������ӿ�
 */
void pm_init(const pm_adapter_t *adt)
{
    pm_ctrl = adt;
}

/**
 * @brief   ���Ĺ���
 * @retval  none
 */
void pm_process(void)
{
    if (!system_is_idle())
        return;
    system_goto_sleep();
}