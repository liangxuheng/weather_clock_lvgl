//只做声明
#ifndef __USART_MY_H__
#define __USART_MY_H__
#include <stdint.h>
struct m_queue;
typedef  struct m_queue* queue_handle;
typedef void (*usart_callback_t)(queue_handle queuex,char c);
//只做声明
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
void usart_init(USART_MY_handle usartx);
void usart_send_string(USART_MY_handle usartx\
	,char*data,uint32_t len);
void usart_send_num(USART_MY_handle usartx,uint32_t num);
void usart_receive_callback_register(USART_MY_handle usartx,usart_callback_t func);
#endif
