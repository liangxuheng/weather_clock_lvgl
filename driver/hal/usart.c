/**
 ******************************************************************************
 * @file    usart.c
 * @brief   USART 驱动：printf 重定向 + 中断接收
 ******************************************************************************
 */

#include "usart_desc.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "cmd_queue.h"
extern USART_MY_handle USART_desc_1;
extern USART_MY_handle USART_desc_2;
static void Usart_pin_af(USART_MY_handle usartx)
{
	if(usartx->usart_num==USART1)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_USART1);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_USART1);
	}
	else if(usartx->usart_num==USART2)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_USART2);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_USART2);
	}
	else if(usartx->usart_num==USART3)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_USART3);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_USART3);
	}
	else if(usartx->usart_num==UART4)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_UART4);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_UART4);
	}
	else if(usartx->usart_num==UART5)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_UART5);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_UART5);
	}
		else if(usartx->usart_num==USART6)
	{
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_rx_pin_source,GPIO_AF_USART6);
		GPIO_PinAFConfig(usartx->Port,usartx->GPIO_tx_pin_source,GPIO_AF_USART6);
	}
}

void usart_init(USART_MY_handle usartx)
{
		if(usartx==NULL)
		{
			return;
		}
		USART_InitTypeDef USART_InStructer;
		memset(&USART_InStructer,0,sizeof(USART_InStructer));
		USART_InStructer.USART_BaudRate=usartx->BaudRate;
		USART_InStructer.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
		USART_InStructer.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
		USART_InStructer.USART_Parity=USART_Parity_No;
		USART_InStructer.USART_StopBits=USART_StopBits_1;
		USART_InStructer.USART_WordLength=USART_WordLength_8b;
		USART_Init(usartx->usart_num,&USART_InStructer);
		
		GPIO_InitTypeDef GPIO_InStructer;
		memset(&GPIO_InStructer,0,sizeof(GPIO_InStructer));
		GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
		GPIO_InStructer.GPIO_Pin=usartx->GPIO_tx_pin;
		GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InStructer.GPIO_Speed=GPIO_Speed_100MHz;
		GPIO_Init(usartx->Port,&GPIO_InStructer);
	
		GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
		GPIO_InStructer.GPIO_Pin=usartx->GPIO_rx_pin;
		GPIO_Init(usartx->Port,&GPIO_InStructer);
		
		//在f4系列中，除了GPIO引脚要配置复用，还要引脚重映射
		Usart_pin_af(usartx);
		
		NVIC_InitTypeDef NVIC_InStructer;
		memset(&NVIC_InStructer,0,sizeof(NVIC_InStructer));
		if(usartx->isinterrupt_rx)
		{
			USART_ITConfig(usartx->usart_num,USART_IT_RXNE,ENABLE);
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
			NVIC_InStructer.NVIC_IRQChannel=usartx->IRQN;
			NVIC_InStructer.NVIC_IRQChannelCmd=ENABLE;
			NVIC_InStructer.NVIC_IRQChannelPreemptionPriority=6;
			NVIC_InStructer.NVIC_IRQChannelSubPriority=0;
			NVIC_Init(&NVIC_InStructer);
		}
		if(usartx->isinterrupt_tx)
		{
			NVIC_SetPriority(usartx->IRQN, NVIC_EncodePriority(NVIC_PriorityGroup_4, 8, 0));
    // 使能中断
    NVIC_EnableIRQ(usartx->IRQN);
		}
		
		USART_Cmd(usartx->usart_num,ENABLE);

}

//内部函数
static void USART_send_data(USART_MY_handle usart,uint32_t data)
{
	USART_SendData(usart->usart_num,(uint16_t)data);
	while(USART_GetFlagStatus(usart->usart_num,USART_FLAG_TXE)==RESET);
}	

void usart_send_string(USART_MY_handle usartx,char*data,uint32_t len)
{
		if(data==NULL||len<=0||usartx==NULL)
		{
			return;
		}
		for(uint32_t i=0;i<len;++i)
		{
			USART_send_data(usartx,data[i]);
		}
}

void usart_send_num(USART_MY_handle usartx,uint32_t num)
{
	if(!usartx)
	{
		return;
	}
	uint32_t temp=num;
	//计算数字的位数
	uint32_t len=0;
	while(temp>0)
	{
		len++;
		temp/=10;
	}
	char buf[len+1];
	memset(buf,0,sizeof(buf));
	for(int32_t i=len-1;i>=0;--i)
	{
		buf[i]=num%10+'0';
		num/=10;
	}
	usart_send_string(usartx,buf,strlen(buf)+1);
}


void usart_receive_callback_register(USART_MY_handle usartx,usart_callback_t func)
{
	if(usartx==USART_desc_1||usartx==USART_desc_2)
	{
		usartx->callback_t=func;
	}
	else 
	{
		usartx->callback_t=NULL;
	}
}

//static uint8_t count=0;
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE))
	{
//		if(count%2)
//		{led_on(led2);count++;}
//		else {led_off(led2);count++;}
//		uint8_t data=USART_ReceiveData(USART1);
//		m_queue_push(debug_queue,data);
		if(USART_desc_1->callback_t)
		{
			uint8_t data=USART_ReceiveData(USART1);
			USART_desc_1->callback_t(debug_queue,data);
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}	



int fputc(int c ,FILE*file)
{
	//告诉编译器该变量是无用而非忘记用
	(void)file;
	USART_SendData(USART1,c);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	return c; 
}
