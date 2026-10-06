#ifndef __CMD_QUEUE_H__
#define __CMD_QUEUE_H__
#include <stdint.h>
#define QUEUE_ERROR -1

struct m_queue;
typedef struct m_queue* queue_handle;

int8_t m_queue_top(queue_handle queuex);
int8_t m_queue_pop(queue_handle queuex);
int8_t m_queue_push(queue_handle queuex, char c);
void m_queue_reset(queue_handle queuex);
void queue_init(queue_handle queuex);

/* USART1 debug 接收队列 */
extern queue_handle debug_queue;

#endif
