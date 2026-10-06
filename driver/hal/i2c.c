#include "stm32f4xx.h"
#include "i2c_desc.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include "delay.h"
//等待标志位的宏函数
#define I2C_CHECK_EVENT(i2cx,EVENT,TIMEOUT)\
do{	uint32_t timeout=TIMEOUT;\
	while(timeout&&(I2C_CheckEvent(i2cx->i2cx,EVENT)==RESET))\
	{\
		delay(10);\
		timeout-=10;\
	}\
	if(timeout<=0)\
	{\
		return false;\
	}\
}while(0);
void m_i2c_init(i2c_handle i2cx)
{
	GPIO_InitTypeDef GPIO_InStructer;
	GPIO_StructInit(&GPIO_InStructer);
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InStructer.GPIO_OType=GPIO_OType_OD;
	GPIO_InStructer.GPIO_Pin=i2cx->GPIO_Pin_scl;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InStructer.GPIO_Speed=GPIO_High_Speed;
	GPIO_Init(i2cx->GPIO_Port_scl,&GPIO_InStructer);
	GPIO_InStructer.GPIO_Pin=i2cx->GPIO_Pin_sda;
	GPIO_Init(i2cx->GPIO_Port_sda,&GPIO_InStructer);
	
	GPIO_PinAFConfig(i2cx->GPIO_Port_scl,i2cx->GPIO_pinSource_scl,i2cx->GPIO_af);
	GPIO_PinAFConfig(i2cx->GPIO_Port_sda,i2cx->GPIO_pinSource_sda,i2cx->GPIO_af);

	I2C_InitTypeDef i2c_InStructer;
	I2C_StructInit(&i2c_InStructer);
	i2c_InStructer.I2C_Ack=i2cx->isack;
	i2c_InStructer.I2C_AcknowledgedAddress=i2cx->widthofaddress;
	i2c_InStructer.I2C_ClockSpeed=i2cx->clockspeed;
	i2c_InStructer.I2C_DutyCycle=i2cx->duty;
	i2c_InStructer.I2C_Mode=i2cx->mode;
	i2c_InStructer.I2C_OwnAddress1=i2cx->ownaddress;
	I2C_Init(i2cx->i2cx,&i2c_InStructer);
	I2C_Cmd(i2cx->i2cx,ENABLE);
}

bool i2c_read(i2c_handle i2cx,uint8_t address_slave, uint8_t address ,uint8_t *buf,uint32_t length)
{
	I2C_AcknowledgeConfig(i2cx->i2cx,ENABLE);
	I2C_GenerateSTART(i2cx->i2cx,ENABLE);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_MODE_SELECT,1000);
	I2C_Send7bitAddress(i2cx->i2cx,address_slave,I2C_Direction_Transmitter);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,1000);
	I2C_SendData(i2cx->i2cx,address);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING,1000);
	
	I2C_AcknowledgeConfig(i2cx->i2cx,ENABLE);
	
	I2C_GenerateSTART(i2cx->i2cx,ENABLE);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_MODE_SELECT,1000);
	I2C_Send7bitAddress(i2cx->i2cx,address_slave,I2C_Direction_Receiver);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED,1000);
	for(uint32_t i=0;i<length;++i)
	{
		//接收最后一个之前先设置发送nack
		if(i==length-1)
		{
			I2C_AcknowledgeConfig(i2cx->i2cx,DISABLE);
		}
		I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_BYTE_RECEIVED,1000);
		buf[i]=I2C_ReceiveData(i2cx->i2cx);
	}
	I2C_GenerateSTOP(i2cx->i2cx,ENABLE);
	I2C_AcknowledgeConfig(i2cx->i2cx,ENABLE);
	return true;
}

bool i2c_write(i2c_handle i2cx,uint8_t address_slave, uint8_t address ,uint8_t *buf,uint32_t length)
{
	I2C_AcknowledgeConfig(i2cx->i2cx,ENABLE);
	I2C_GenerateSTART(i2cx->i2cx,ENABLE);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_MODE_SELECT,1000);
	I2C_Send7bitAddress(i2cx->i2cx,address_slave,I2C_Direction_Transmitter);
	I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED,1000);
	
	I2C_SendData(i2cx->i2cx, address);
	I2C_CHECK_EVENT(i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING, 1000);
	for(uint32_t i=0;i<length;++i)
	{
		I2C_SendData(i2cx->i2cx,buf[i]);
		I2C_CHECK_EVENT(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING,1000);
	}
	I2C_GenerateSTOP(i2cx->i2cx,ENABLE);
	return true;
}
