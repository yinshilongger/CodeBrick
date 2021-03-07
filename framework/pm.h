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
#ifndef _PM_H_
#define _PM_H_

#include "comdef.h"
#include <stdbool.h>

#define __pm_item_register(name, idle, suspend, resume)\
    USED ANONY_TYPE(const pm_item_t,__pm_item_##suspend)\
    SECTION("pm.item.1") = {name, idle, suspend, resume}

typedef struct {
    /**
     * @brief   �豸����
     */    
    const char *name;                      
    /**
     * @brief   ��ǰ�豸����״̬(ֻ�����е�pm_item_t����������,ϵͳ��������)
     * @note    �����NULL,���ʾ����ϵͳ����
     * @retval  true - ����״̬��������,false - ����ģʽ,����������
     */
    bool   (*idle)(void);
    /**
     * @brief      ����֪ͨ
     * @retval     �´λ���ʱ��(��λ:ms, 0 - ��ʾ����ϵͳ����)
     */    
    unsigned int  (*sleep_notify)(void);
    /**
     * @brief     ����֪ͨ
     * @retval  none
     */    
    void  (*wakeup_notify)(void);
}pm_item_t;

/*�͹��������� ---------------------------------------------------------------*/
typedef struct {
    /**
     * @brief    ϵͳ�������ʱ��(ms)
     */  
    unsigned int max_sleep_time;
    /**
     * @brief     ��������״̬
     * @param[in] time - �ڴ�����ʱ��(ms)
     * @retval    ʵ������ʱ��
     * @note      ����֮����Ҫ������������,1������Ҫ��ʱ������ι���Ź�,�����������
     *            �ڼ䷢������.����һ����������Ҫ��������ʱ���ϵͳ�δ�ʱ��,�����
     *            ���ʱ�䲻׼��
     */     
    unsigned int (*goto_sleep)(unsigned int time);
}pm_adapter_t;

/**
 * @brief     ���Ĺ�����ע��
 * @param[in] name    - ��Ŀ����
 * @param[in] idle    - ָʾ�豸�Ƿ����,�����NULL,���ʾ����ϵͳ����
 * @param[in] sleep_notify   - ����֪ͨ,����Ҫ����NULL
 * @param[in] wakeup_notify  - ����֪ͨ,����Ҫ����NULL
 */ 
#define pm_dev_register(name, idle, sleep_notify, wakeup_notify)\
__pm_item_register(name, idle, sleep_notify, wakeup_notify)


void pm_init(const pm_adapter_t *adt);

void pm_enable(void);

void pm_disable(void);

void pm_process(void);


#endif

