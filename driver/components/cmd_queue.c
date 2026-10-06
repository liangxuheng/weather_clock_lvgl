/**
 ******************************************************************************
 * @file    cmd_queue.c
 * @brief   命令队列：串口数据环形缓冲区，解析后分发到 UI
 ******************************************************************************
 */

#include "queue_desc.h"
#include "cmd_queue.h"
#include <stdlib.h>
#include <string.h>

static struct m_queue debug_queue_buf;
queue_handle debug_queue = &debug_queue_buf;

/**
 * @brief  初始化命令环形队列
 * @param  queuex  队列句柄
 * @retval None
 */
void queue_init(queue_handle queuex)
{
	memset(queuex, 0, sizeof(struct m_queue));
}

/**
 * @brief  出队一个字节
 * @param  queuex  队列句柄
 * @retval 0=成功, -1=空
 */
int8_t m_queue_pop(queue_handle queuex)
{
	if (queuex == NULL || queuex->front == queuex->rear)
	{
		return QUEUE_ERROR;
	}
	queuex->front = (queuex->front + 1) % QUEUE_SIZE;
	return 0;
}

/**
 * @brief  查看队首字节
 * @param  queuex  队列句柄
 * @retval 队首字节, -1=空
 */
int8_t m_queue_top(queue_handle queuex)
{
	if (queuex == NULL || queuex->front == queuex->rear)
	{
		return QUEUE_ERROR;
	}
	int8_t temp = queuex->buf[queuex->front];
	return temp;
}

/**
 * @brief  入队一个字节
 * @param  queuex  队列句柄
 * @param  c       字节
 * @retval 0=成功, -1=满
 */
int8_t m_queue_push(queue_handle queuex, char c)
{
	if (queuex == NULL || ((queuex->rear + 1) %
		(sizeof(queuex->buf) / sizeof(char)) == queuex->front))
	{
		return QUEUE_ERROR;
	}
	queuex->buf[queuex->rear] = c;
	queuex->rear = (queuex->rear + 1) % QUEUE_SIZE;
	return 0;
}

/**
 * @brief  清空队列
 * @param  queuex  队列句柄
 * @retval None
 */
void m_queue_reset(queue_handle queuex)
{
	for (volatile char* p = queuex->buf; p != queuex->buf + sizeof(queuex->buf); ++p)
	{
		*p = 0;
	}
	queuex->front = 0;
	queuex->rear = 0;
}
