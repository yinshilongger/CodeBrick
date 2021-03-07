/******************************************************************************
 * @brief    按键任务(演示key模块使用)
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

static key_t key;                                    /*按键定义*/

static void key_event(int type, unsigned int duration);

/* 
 * @brief       读取按键电平状态
 * @return      0 | 1
 */ 
static int readkey(void)
{
    return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0;
}

/*
 * @brief	   按键中断处理
 */
void EXTI0_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line0);    
}

/* 
 * @brief       按键 io初始化
 *              PC0 -> key;
 * @return      none
 */ 
static void key_io_init(void)
{
    /* Enable GPIOA clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    gpio_conf(GPIOC, GPIO_Mode_IN, GPIO_PuPd_UP, GPIO_Pin_0);
    
    //低功耗模式下,为了能够检测到按键，配置为中断唤醒
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);
    exti_conf(EXTI_Line0, EXTI_Trigger_Falling, ENABLE);
    nvic_conf(EXTI0_IRQn, 0x0F, 0x0F);
    
    key_create(&key, readkey, key_event);            /*创建按键*/
}

/* 
 * @brief       按键事件处理
 * @return      type - 
 */
static void key_event(int type, unsigned int duration)
{
    if (type == KEY_PRESS)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_SLOW, 3);  /*短按,绿灯闪3下*/
    else if (type == KEY_LONG_DOWN)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_ON, 0);    /*长按,绿灯常亮*/
    else if (type == KEY_LONG_UP)
        led_ctrl(LED_TYPE_GREEN, LED_MODE_OFF, 0);   /*长按后释放,绿灯关闭*/
}

driver_init("key", key_io_init);                     /*按键初始化*/
task_register("key", key_scan_process, 20);          /*按键扫描任务, 20ms轮询1次*/

/* 低功耗管理 -----------------------------------------------------------------*/
#include "pm.h"                                     

/*
 * @brief	   休眠通知
 */
static unsigned int  key_sleep_notify(void)
{
    return (key_busy(&key) || readkey()) ? 20 : 0;    /* 非空闲时20ms要唤醒1次*/
} pm_dev_register("key", NULL, key_sleep_notify, NULL);
