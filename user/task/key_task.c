/******************************************************************************
 * @brief    ��������(��ʾkeyģ��ʹ��)
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2020/07/03     roger.luo    
 ******************************************************************************/
#include "led.h"
#include "key.h"
#include "module.h"
#include "public.h"
#include <stddef.h>

static key_t key;                                    /*��������*/

static void key_event(int type, unsigned int duration);

/* 
 * @brief       ��ȡ������ƽ״̬
 * @return      0 | 1
 */ 
static int readkey(void)
{
    return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0;
}

/*
 * @brief	   �����жϴ���
 */
void EXTI0_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line0);    
}

/* 
 * @brief       ���� io��ʼ��
 *              PC0 -> key;
 * @return      none
 */ 
static void key_io_init(void)
{
    /* Enable GPIOA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    gpio_conf(GPIOC, GPIO_Mode_IN, GPIO_PuPd_UP, GPIO_Pin_0);
    
    //�͹���ģʽ��,Ϊ���ܹ���⵽����������Ϊ�жϻ���
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);
    exti_conf(EXTI_Line0, EXTI_Trigger_Falling, ENABLE);
    nvic_conf(EXTI0_IRQn, 0x0F, 0x0F);
    
    key_create(&key, readkey, key_event);            /*��������*/
}

/* 
 * @brief       �����¼�����
 * @return      type - 
 */
static void key_event(int type, unsigned int duration)
{
    if (type == KEY_PRESS)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_SLOW, 3);  /*�̰�,�̵���3��*/
    else if (type == KEY_LONG_DOWN)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_ON, 0);    /*����,�̵Ƴ���*/
    else if (type == KEY_LONG_UP)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_OFF, 0);   /*�������ͷ�,�̵ƹر�*/
}

driver_init("key", key_io_init);                     /*������ʼ��*/
task_register("key", key_scan_process, 20);          /*����ɨ������, 20ms��ѯ1��*/

/* �͹��Ĺ��� -----------------------------------------------------------------*/
#include "pm.h"                                     

/*
 * @brief	   ����֪ͨ
 */
static unsigned int  key_sleep_notify(void)
{
    return (key_busy(&key) || readkey()) ? 20 : 0;    /* �ǿ���ʱ20msҪ����1��*/
} pm_dev_register("key", NULL, key_sleep_notify, NULL);
