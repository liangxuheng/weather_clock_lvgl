#ifndef __LCD24_DESC_H_
#define __LCD24_DESC_H_
#include "stm32f4xx.h"
#include <stdint.h>
#define LCD24_WIDTH 240
#define LCD24_HEIGHT 320
struct spi_Struct;
typedef struct spi_Struct* spi_handler;
struct lcd24
{
	spi_handler spi_x;

	GPIO_TypeDef*GPIO_RESET_PORT;
	uint16_t GPIO_RESET_PIN;
	GPIO_TypeDef*GPIO_DC_PORT;
	uint16_t GPIO_DC_PIN;
};
#endif
