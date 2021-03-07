/******************************************************************************
 * @brief    设备相关操作
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 *
 ******************************************************************************/

#ifndef _PLATFORM_DEVICE_H_
#define _PLATFORM_DEVICE_H_

#include <stdbool.h>

#define LOWPOWER_MODE                    1               /* 使能低功耗模式*/
#define SYS_TICK_INTERVAL                10              /* 系统滴答时间(ms) */    
#define MAX_DOG_FEED_TIME                10 * 1000       /* 最大看门狗喂狗时间*/
     
     
bool is_timeout(unsigned int start, unsigned int timeout);

#endif
