/******************************************************************************
 * @brief    tty串口打印驱动
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2015-07-03     Morro        初版
 * 2021-03-07     Morro        增加发送缓冲区空判断
 ******************************************************************************/

#ifndef	_TTY_H_
#define	_TTY_H_

#define TTY_RXBUF_SIZE		 256
#define TTY_TXBUF_SIZE		 1024

/*接口声明 --------------------------------------------------------------------*/
typedef struct {
    void (*init)(int baudrate);                                   
    unsigned int (*write)(const void *buf, unsigned int len);    
    unsigned int (*read)(void *buf, unsigned int len);           
    bool (*tx_isfull)(void);                                    /*发送缓冲区满*/
    bool (*tx_isempty)(void);                                   /*发送缓冲区空*/
    bool (*rx_isempty)(void);                                   /*接收缓冲区空*/
}tty_t;

extern const tty_t tty;

#endif
