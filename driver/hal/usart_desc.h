//存放USART类
#ifndef __USART_DESC_H__MY__
#define __USART_DESC_H__MY__
#include "stm32f4xx.h"
//USART_InitTypeDef 
//GPIO_InitTypeDef
//USART_Init()
struct m_queue;
typedef  struct m_queue* queue_handle;
typedef void  (*usart_callback_t)(queue_handle queuex,char c);
struct USART_desc
{
	uint16_t GPIO_tx_pin_source;
	uint16_t GPIO_rx_pin_source;
	USART_TypeDef* usart_num;//开启串口的串口号
	uint32_t BaudRate;//串口波特率
	GPIO_TypeDef* Port;//端口
	uint32_t GPIO_tx_pin;//发送引脚
	uint32_t GPIO_rx_pin;//接收引脚
	uint8_t isinterrupt_tx;//是否开启发送中断
	uint8_t isinterrupt_rx;//是否开启接收中断
	usart_callback_t callback_t;//串口中断的回调函数
	uint8_t IRQN;//中断通道
};
#endif
