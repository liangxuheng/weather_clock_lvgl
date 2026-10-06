//内部结构体的实现
#ifndef __LED_DESC_H__
#define __LED_DESC_H__
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
struct led_desc
{
	GPIO_TypeDef * GPIO_PORT;//端口
	uint16_t GPIO_PIN;//引脚
	BitAction on;//导通电平
	BitAction off;//断开电平
};
#endif
