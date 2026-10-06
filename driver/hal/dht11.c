/**
 ******************************************************************************
 * @file    dht11.c
 * @brief   DHT11 温湿度传感器驱动：单总线时序读取
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include "dht11.h"
#include "dht11_desc.h"
#include "delay.h"
#include "timer.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#define DATA_BITS 40
#define ERROR_NUM -1
//extern volatile uint32_t tick;
/**
 * @brief  DHT11初始化：配置GPIO
 * @param  dht11  DHT11句柄
 * @retval None
 */
void dht11_init(dth_handle dht11)
{
	//printf("SystemCoreClock = %lu\n", SystemCoreClock);
	GPIO_InitTypeDef GPIO_InStructer;
	GPIO_StructInit(&GPIO_InStructer);
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InStructer.GPIO_OType=GPIO_OType_OD;
	GPIO_InStructer.GPIO_Pin=dht11->GPIO_Pin;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(dht11->GPIO_Port,&GPIO_InStructer);
	//等待超过一秒的时间让外设稳定
	//delay_s(2);
	vTaskDelay(pdMS_TO_TICKS(2000));
	//printf("init finish\r\n");
}
static void dht11_generate_start(dth_handle dht11)
{
	//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
	//先设置成输出
	GPIO_InitTypeDef GPIO_InStructer;
	GPIO_StructInit(&GPIO_InStructer);
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InStructer.GPIO_OType=GPIO_OType_OD;
	GPIO_InStructer.GPIO_Pin=dht11->GPIO_Pin;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(dht11->GPIO_Port,&GPIO_InStructer);
	GPIO_WriteBit(dht11->GPIO_Port,dht11->GPIO_Pin,Bit_RESET);
	//低电平保持时间不能小于18ms
	//delay_ms(30);
	vTaskDelay(pdMS_TO_TICKS(30));
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_IN;
	GPIO_Init(dht11->GPIO_Port,&GPIO_InStructer);
	//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
}

static int8_t dht11_databit_read(dth_handle dht11)
{
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
		//先等待50us的高电平
		uint64_t start;
//			while(GPIO_ReadInputDataBit(dht11->GPIO_Port,dht11->GPIO_Pin)==Bit_SET)
//		{
//				if(tick-start>250)
//				{
//					return ERROR_NUM;
//				}
//		}
		start=tick_get_us();
		while(GPIO_ReadInputDataBit(dht11->GPIO_Port,dht11->GPIO_Pin)==Bit_RESET)
		{
				if(tick_get_us()-start>100)
				{
					return ERROR_NUM;
				}
		}
		uint64_t last;
		last=tick_get_us();
		//等待低电平
		while(GPIO_ReadInputDataBit(dht11->GPIO_Port,dht11->GPIO_Pin)==Bit_SET)
		{		

			if(tick_get_us()-last>100)
			{
				return ERROR_NUM;
			}
		}
			if(tick_get_us()-last<=50)
		{
			//printf("%d\r\n",tick);
			return Bit_RESET;
		}
		else{
			//printf("%d\r\n",tick);
			return Bit_SET;
		}

		//根据低电平的时间判断
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);

}

static bool dht11_wait_for_recv(dth_handle dht11)
{
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
	uint64_t timerout=tick_get_us();
	   while(GPIO_ReadInputDataBit(dht11->GPIO_Port, dht11->GPIO_Pin) == Bit_SET)
    {
        if(tick_get_us() - timerout > 100)
            return false;
    }
    
    timerout = tick_get_us();
	while(GPIO_ReadInputDataBit(dht11->GPIO_Port,dht11->GPIO_Pin)==Bit_RESET){
		//printf("%d\r\n",tick);
	if(tick_get_us()-timerout>100)
	{
		//printf("%d\r\n",tick);
	return false;
	}
	}
	timerout=tick_get_us();
	while(GPIO_ReadInputDataBit(dht11->GPIO_Port,dht11->GPIO_Pin)==Bit_SET){
		//printf("%d\r\n",tick);
	if(tick_get_us()-timerout>100)
	{
		//printf("%d\r\n",tick);
		return false;
	}
	}
	return true;
}

/**
 * @brief  读取DHT11温湿度数据
 * @param  dht11   DHT11句柄
 * @param  buf     接收缓冲区
 * @param  length  读取长度
 * @retval true=成功
 */
bool dht11_data_read(dth_handle dht11,uint8_t *buf,uint8_t length)
{
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
		dht11_generate_start(dht11);
		if(!dht11_wait_for_recv(dht11))
		{
			return false;
		}
		uint8_t len=length<DATA_BITS/8?length:DATA_BITS/8;
		for(uint8_t i=0;i<len;++i)
		{
			for(uint8_t j=0;j<sizeof(uint8_t)*8;++j)
			{
				int8_t data=dht11_databit_read(dht11);
				if(data==ERROR_NUM)
				{
					return false;
				}
				buf[i]|=((data&0x1)<<(8-j-1));
			}
		}
		uint8_t sum=0;
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
		for(uint8_t i=0;i<len-1;++i)
		{
			sum+=buf[i];
		}
		//printf("%s %d %s\r\n",__FILE__,__LINE__,__FUNCTION__);
		if(sum!=buf[len-1])
		{
			return false;
		}
		else 
		{
			return true;
		}
}
