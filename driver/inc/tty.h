/******************************************************************************
 * @brief    tty���ڴ�ӡ����
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2015-07-03     Morro        ����
 * 2021-03-07     Morro        ���ӷ��ͻ��������ж�
 ******************************************************************************/

#ifndef	_TTY_H_
#define	_TTY_H_

#define TTY_RXBUF_SIZE		 256
#define TTY_TXBUF_SIZE		 1024

/*�ӿ����� --------------------------------------------------------------------*/
typedef struct {
    void (*init)(int baudrate);                                   
    unsigned int (*write)(const void *buf, unsigned int len);    
    unsigned int (*read)(void *buf, unsigned int len);           
    bool (*tx_isfull)(void);                                    /*���ͻ�������*/
    bool (*tx_isempty)(void);                                   /*���ͻ�������*/
    bool (*rx_isempty)(void);                                   /*���ջ�������*/
}tty_t;

extern const tty_t tty;

#endif
