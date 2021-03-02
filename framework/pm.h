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
     * @retval  true - ����״̬��������,false - ����ģʽ,����������
     */
    bool   (*idle)(void);
    /**
     * @brief      �����豸
     * @param[out] next_wakup_time - �´λ���ʱ��
     * @retval     ����ʱ��(��λ:ms, 0 - ��ʾ����ϵͳ����)
     */    
    void  (*suspend)(unsigned int *next_wakup_time);
    /**
     * @brief   �ָ�
     * @retval  none
     */    
    void  (*resume)(void);
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
     */     
    void (*goto_sleep)(unsigned int time);
}pm_adapter_t;

/**
 * @brief     ���Ĺ�����ע��
 * @param[in] name    - ��Ŀ����
 * @param[in] idle    - ָʾ�豸�Ƿ����
 * @param[in] suspend - ϵͳ����֪ͨ
 * @param[in] resume  - ����֪ͨ
 */ 
#define pm_dev_register(name, idle, suspend, resume)\
__pm_item_register(name, idle, suspend, resume)


void pm_init(const pm_adapter_t *adt);

void pm_process(void);


#endif