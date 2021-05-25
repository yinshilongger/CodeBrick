/******************************************************************************
 * @brief        �첽��ҵ
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2020-09-22     Morro        Initial version. 
 ******************************************************************************/
#include "async_work.h"
#include "comdef.h"
#include <stdbool.h>

/*******************************************************************************
 * @brief       ��ҵ���
 * @param[in]   qlink     - ������
 * @param[in]   new_item -  ������
 * @return      none
 ******************************************************************************/
static inline void workqueue_put(struct qlink *q, work_node_t *n)
{
	qlink_put(q, &n->node);                                  /*���뵽��������*/
}


/*******************************************************************************
 * @brief       ��ҵ����
 * @param[in]   qlink     - ������
 * @return      ������
 ******************************************************************************/
static inline work_node_t *workqueue_get(struct qlink *q)
{
	struct qlink_node *n = qlink_get(q);
	return n ? container_of(n, work_node_t, node) : NULL;
}

/*******************************************************************************
 * @brief       ��ҵԤ����
 * @param[in]   qlink     - ������
 * @return      ������
 ******************************************************************************/
static inline work_node_t * workqueue_peek(struct qlink *q)
{
	struct qlink_node *n = qlink_peek(q);
	return n ? container_of(n, work_node_t, node) : NULL;
}


/*
 * @brief       �쳣��ҵ��ʼ��
 * @param[in]   w        - ��ҵ������
 * @param[in]   node_tbl - ��ҵ�ڵ��
 * @param[in]   count    - node_tbl����
 */
void async_work_init(async_work_t *w, work_node_t *node_tbl, int count)
{
    qlink_init(&w->idle);
    qlink_init(&w->ready);
    while (count--) {
        qlink_put(&w->idle, &node_tbl->node);
        node_tbl++;
    }
}

/*
 * @brief       ������ҵ������
 * @param[in]   w        - ��ҵ������
 * @param[in]   params   - ��ҵ����
 * @param[in]   work     - ��ҵ���
 */
bool async_work_add(async_work_t *w, void *object, void *params, 
                    async_work_func_t work)
{
    work_node_t *n = workqueue_get(&w->idle);
    if (n == NULL)
        return false;
    n->object = object;
    n->params = params;
    n->work   = work;
    
    workqueue_put(&w->ready, n);               /*���뵽��������*/          
    return true;
}


/*
 * @brief       �첽��ҵ����
 * @param[in]   w         - ��ҵ������
 */
void async_work_process(async_work_t *w)
{
    work_node_t *n;
    if ((n = workqueue_get(&w->ready)) == NULL)
        return;
    n->work(w, n->object, n->params);
    workqueue_put(&w->idle, n);
}

