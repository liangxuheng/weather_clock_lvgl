#ifndef __dht11_DESC_H_
#define __dht11_DESC_H_
#include "stm32f4xx.h"
#include <stdint.h>
struct dht11_struct
{
	GPIO_TypeDef * GPIO_Port;
	uint16_t GPIO_Pin;
};
#endif
