/******************************************************************************
 * @brief    平台相关初始化(带低功耗管理版本)
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ******************************************************************************/
#include "module.h"
#include "public.h"
#include "config.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "tty.h"
#include "pm.h"

/*
 * @brief	   RTC唤醒标志
 */
static volatile bool is_rtc_wakekup = false;
/*
 * @brief	   系统空闲时间(用于功耗控制)
 */
static unsigned int  system_idle_time = 0;

/*
 * @brief	   rtc唤醒中断
 * @param[in]  none
 * @return 	   none
 */
void RTC_WKUP_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI_ClearITPendingBit(EXTI_Line22);
        systick_increase(SYS_TICK_INTERVAL);
        is_rtc_wakekup = true;
    }
}

/*
 * @brief	   RTC初始化
 */
void rtc_init(void)
{
    volatile unsigned int retry = 0;
    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    do{                           
        retry++;
    } while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && retry < 20000);
    
    if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET) {
        /* Select the RTC Clock Source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);        
    } else {
        RCC_LSEConfig(RCC_LSE_OFF);
        /* 外部晶振启动不了,使用内部的 */ 
        RCC_LSICmd(ENABLE);  
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET){ }    
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    }

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
}

/*
 * @brief	   rtc唤醒配置
 * @param[in]  none(1~4000ms)
 */
void rtc_wakeup_config(unsigned int ms)
{       
    RTC_WakeUpCmd(DISABLE);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);
    RTC_SetWakeUpCounter((unsigned int )((ms * 16384.0 / 1000.0) + 0.5 ) - 1); 

    exti_conf(EXTI_Line22, EXTI_Trigger_Rising, ENABLE);    
    nvic_conf(RTC_WKUP_IRQn, 0, 0);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    RTC_WakeUpCmd(ENABLE);
}

/**
  * @brief  从停止模式中唤醒后,打开HSE，PLL    
  */
static void system_wakup_config(void)
{
    volatile unsigned int retry = 0;
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET && ++retry < 200000)
    {
    }
    /* Enable PLL */
    RCC_PLLCmd(ENABLE);
    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08)
    {
    }
}

/*
 * @brief	   系统休眠接口实现
 * @param[in]  ms  - 休眠时长
 * @note       休眠之后需要考虑两件事情,1个是需要定时起来给喂看门狗,否则会在休眠期
 *             间发送重启.另外一件事情是需要补偿休眠时间给系统滴答时钟,否则会造成
 *             系统时钟不准.
 * @return 	   实际休眠时间           
 */
static unsigned int system_sleep(unsigned int ms)
{    
    unsigned int start_time = get_tick();
    /**
      一般来说进入低功耗应根据sleep_time指定时长使用rtc_wakeup_config配置下一次唤
      醒时间,这种方式可以让系统休眠时间尽量拉长，功耗也最低。但是对于STM32F4系列MCU
      来说,低功耗模式下除了RTC其它时钟都是停止的,没有像L4一样拥有低功耗定时器。如果
      突然外部中断唤醒，系统滴答时钟并不好做补偿处理。故对于F4系列MCU，这里提供一个
      不需要时间补偿的处理方式，系统只在初始化时配置1次[rtc_wakeup_config],即系统以
      最小休眠时间为SYS_TICK_INTERVAL(默认10ms)为单位间歇运行.无论系统是否被外部中
      断唤醒都不会影响系统滴答时钟精度,缺点由于唤醒频繁,比理论极限功耗稍大
     */
    while (get_tick() - start_time < ms) {
        is_rtc_wakekup = false;
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        if (!is_rtc_wakekup)                              //外部中断唤醒
            break;
    }
    system_wakup_config();                                //启动HSE
    //printf("Sleep Time:%d isrtc:%d...\r\n\r\n", get_tick() - start_time, is_rtc_wakekup);
    return get_tick() - start_time;
}


/*
 * @brief	   电源管理适配器
 */
static const pm_adapter_t pm_adapter = {
    MAX_DOG_FEED_TIME * 8 / 10,                           //确保能提前唤醒并喂狗
    system_sleep
};

/*
 * @brief	   硬件驱动初始化
 * @param[in]   none
 * @return 	    none
 */
static void bsp_init(void)
{    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
    tty.init(115200);
    SystemCoreClockUpdate();
    RCC_APB2PeriphClockLPModeCmd(RCC_APB2Periph_SYSCFG,ENABLE);
    rtc_init();
    rtc_wakeup_config(SYS_TICK_INTERVAL);               //使用RTC驱动系统
    wdog_conf(MAX_DOG_FEED_TIME);                       //初始化看门狗
    pm_init(&pm_adapter);                               //初始化低功耗管理器
    pm_enable();                                        //启用低功耗管理
    
    //
    //低功耗模式下,为了能够正常接收数据,配置串口1 RX为唤醒中断
    //
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);    
    exti_conf(EXTI_Line10, EXTI_Trigger_Falling, ENABLE); 
    nvic_conf(EXTI15_10_IRQn, 5, 1);
    
}system_init("bsp", bsp_init); 

/*
 * @brief	   重定向printf
 */
int fputc(int c, FILE *f)
{       
    tty.write(&c, 1);
    while (tty.tx_isfull()) {}                           //防止丢LOG
    return c;
}

/*
 * @brief	   串口接收中断处理
 */
void EXTI15_10_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line10);
    EXTI_ClearITPendingBit(EXTI_Line8);
    system_idle_time = get_tick();                       //防止接收数据过程中休眠      
}

/*
 * @brief	   默认开机或者串口有通信活动,3S内不允许进入低功耗
 */
static bool system_is_idle(void)
{
    return is_timeout(system_idle_time, 3000) && tty.rx_isempty() && tty.tx_isempty();
}pm_dev_register("sys", system_is_idle, NULL, NULL);


/*
 * @brief	   功耗管理任务
 */
static void pm_task(void)
{
    pm_process();
}task_register("pm", pm_task, 0); 


/*
 * @brief	   喂狗任务
 */
static void wdog_task(void)
{
    IWDG_ReloadCounter();
}task_register("dog", wdog_task, 1000); 
