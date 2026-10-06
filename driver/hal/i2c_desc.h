#ifndef __I2C_DESC_H_
#define __I2C_DESC_H_
#include "stm32f4xx.h"
#include <stdint.h>
struct my_i2c{
		I2C_TypeDef* i2cx;
		uint32_t clockspeed;//时钟速度
		uint16_t mode;//i2c通信模式
		uint16_t duty;//高速模式下的占空比
		uint16_t ownaddress;//自己作为从机时的从机地址
		uint16_t isack;//是否发送ack
		uint16_t widthofaddress;//地址的宽度
		GPIO_TypeDef * GPIO_Port_scl;//复用端口
		uint16_t GPIO_Pin_scl;//复用引脚
		GPIO_TypeDef * GPIO_Port_sda;//复用端口
		uint16_t GPIO_Pin_sda;//复用引脚
		uint16_t GPIO_pinSource_scl;
		uint16_t GPIO_pinSource_sda;
		uint8_t GPIO_af;
};
#endif
