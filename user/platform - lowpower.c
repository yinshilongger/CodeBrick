/******************************************************************************
 * @brief    ƽ̨��س�ʼ��(���͹��Ĺ���汾)
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
 * @brief	   RTC���ѱ�־
 */
static volatile bool is_rtc_wakekup = false;
/*
 * @brief	   ϵͳ����ʱ��(���ڹ��Ŀ���)
 */
static unsigned int  system_idle_time = 0;

/*
 * @brief	   rtc�����ж�
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
 * @brief	   RTC��ʼ��
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
        /* �ⲿ������������,ʹ���ڲ��� */ 
        RCC_LSICmd(ENABLE);  
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET){ }    
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    }

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
}

/*
 * @brief	   rtc��������
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
  * @brief  ��ֹͣģʽ�л��Ѻ�,��HSE��PLL    
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
 * @brief	   ϵͳ���߽ӿ�ʵ��
 * @param[in]  ms  - ����ʱ��
 * @note       ����֮����Ҫ������������,1������Ҫ��ʱ������ι���Ź�,�������������
 *             �䷢������.����һ����������Ҫ��������ʱ���ϵͳ�δ�ʱ��,��������
 *             ϵͳʱ�Ӳ�׼.
 * @return 	   ʵ������ʱ��           
 */
static unsigned int system_sleep(unsigned int ms)
{    
    unsigned int start_time = get_tick();
    /**
      һ����˵����͹���Ӧ����sleep_timeָ��ʱ��ʹ��rtc_wakeup_config������һ�λ�
      ��ʱ��,���ַ�ʽ������ϵͳ����ʱ�価������������Ҳ��͡����Ƕ���STM32F4ϵ��MCU
      ��˵,�͹���ģʽ�³���RTC����ʱ�Ӷ���ֹͣ��,û����L4һ��ӵ�е͹��Ķ�ʱ�������
      ͻȻ�ⲿ�жϻ��ѣ�ϵͳ�δ�ʱ�Ӳ����������������ʶ���F4ϵ��MCU�������ṩһ��
      ����Ҫʱ�䲹���Ĵ���ʽ��ϵͳֻ�ڳ�ʼ��ʱ����1��[rtc_wakeup_config],��ϵͳ��
      ��С����ʱ��ΪSYS_TICK_INTERVAL(Ĭ��10ms)Ϊ��λ��Ъ����.����ϵͳ�Ƿ��ⲿ��
      �ϻ��Ѷ�����Ӱ��ϵͳ�δ�ʱ�Ӿ���,ȱ�����ڻ���Ƶ��,�����ۼ��޹����Դ�
     */
    while (get_tick() - start_time < ms) {
        is_rtc_wakekup = false;
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        if (!is_rtc_wakekup)                              //�ⲿ�жϻ���
            break;
    }
    system_wakup_config();                                //����HSE
    //printf("Sleep Time:%d isrtc:%d...\r\n\r\n", get_tick() - start_time, is_rtc_wakekup);
    return get_tick() - start_time;
}


/*
 * @brief	   ��Դ����������
 */
static const pm_adapter_t pm_adapter = {
    MAX_DOG_FEED_TIME * 8 / 10,                           //ȷ������ǰ���Ѳ�ι��
    system_sleep
};

/*
 * @brief	   Ӳ��������ʼ��
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
    rtc_wakeup_config(SYS_TICK_INTERVAL);               //ʹ��RTC����ϵͳ
    wdog_conf(MAX_DOG_FEED_TIME);                       //��ʼ�����Ź�
    pm_init(&pm_adapter);                               //��ʼ���͹��Ĺ�����
    pm_enable();                                        //���õ͹��Ĺ���
    
    //
    //�͹���ģʽ��,Ϊ���ܹ�������������,���ô���1 RXΪ�����ж�
    //
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);    
    exti_conf(EXTI_Line10, EXTI_Trigger_Falling, ENABLE); 
    nvic_conf(EXTI15_10_IRQn, 5, 1);
    
}system_init("bsp", bsp_init); 

/*
 * @brief	   �ض���printf
 */
int fputc(int c, FILE *f)
{       
    tty.write(&c, 1);
    while (tty.tx_isfull()) {}                           //��ֹ��LOG
    return c;
}

/*
 * @brief	   ���ڽ����жϴ���
 */
void EXTI15_10_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line10);
    EXTI_ClearITPendingBit(EXTI_Line8);
    system_idle_time = get_tick();                       //��ֹ�������ݹ���������      
}

/*
 * @brief	   Ĭ�Ͽ������ߴ�����ͨ�Ż,3S�ڲ��������͹���
 */
static bool system_is_idle(void)
{
    return is_timeout(system_idle_time, 3000) && tty.rx_isempty() && tty.tx_isempty();
}pm_dev_register("sys", system_is_idle, NULL, NULL);


/*
 * @brief	   ���Ĺ�������
 */
static void pm_task(void)
{
    pm_process();
}task_register("pm", pm_task, 0); 


/*
 * @brief	   ι������
 */
static void wdog_task(void)
{
    IWDG_ReloadCounter();
}task_register("dog", wdog_task, 1000); 
