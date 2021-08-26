/******************************************************************************
 * @brief    ϵͳģ�����(����ϵͳ��ʼ��,ʱ��Ƭ��ѯϵͳ)
 *
 * Copyright (c) 2017~2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2016-06-24     Morro        �������
 * 2020-05-23     Morro        ������������,��ֹģ����������
 * 2020-06-28     Morro        ����is_timeout��ʱ�жϽӿ�
 * 2020-09-28     Morro        ���α������δ��ʼ��timer�����������˿�ָ������⣡
 *                             
 ******************************************************************************/
#include "module.h"


static volatile unsigned int tick;               //ϵͳ�δ��ʱ

/*
 * @brief   ����ϵͳ������(��ʱ���ж��е���,1ms 1��)
 */
void systick_increase(unsigned int ms)
{
	tick += ms;
}

/*
 * @brief       ��ȡϵͳ�δ�ʱ��ֵ(ͨ����λ��1ms)
 */
unsigned int get_tick(void)
{
	return tick;
}

/*
 * @brief       ��ʱ�ж�
 * @param[in]   start   - ��ʼʱ��
 * @param[in]   timeout - ��ʱʱ��(ms)
 */
bool is_timeout(unsigned int start, unsigned int timeout)
{
    return get_tick() - start > timeout;
}

/*
 * @brief       �մ���,���ڶ�λ�����
 */
static void nop_process(void) {}
    
//��һ����ʼ����
const init_item_t init_tbl_start SECTION("init.item.0") = {     
    "", nop_process
};
//������ʼ����
const init_item_t init_tbl_end SECTION("init.item.4") = {       
    "", nop_process
};

//��һ��������
const task_item_t task_tbl_start SECTION("task.item.0") = {     
    "", nop_process
};
//����������
const task_item_t task_tbl_end SECTION("task.item.2") = {       
    "", nop_process
};

/*
 * @brief       ģ���ʼ����
 *              ��ʼ��ģ���Ż��� system_init > driver_init > module_init
 * @param[in]   none
 * @return      none
 */
void module_task_init(void)
{
    const init_item_t *it = &init_tbl_start;
    while (it < &init_tbl_end) {
        it++->init();
    }
}

/*
 * @brief       ������ѯ����
 * @param[in]   none
 * @return      none
 */
void module_task_process(void)
{
    const task_item_t *t;
    for (t = &task_tbl_start + 1; t < &task_tbl_end; t++) {
        if  ((get_tick() - *t->timer) >= t->interval) {
            *t->timer = get_tick();
            t->handle();
        }
    }
}
