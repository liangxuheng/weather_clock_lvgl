#ifndef __SPI_DESC_H_
#define __SPI_DESC_H_
#include "stm32f4xx.h"
#include <stdint.h>
struct spi_Struct
{
	GPIO_TypeDef *GPIO_MOSI_PORT;
	uint16_t GPIO_MOSI_PIN;
	uint16_t GPIO_MOSI_PIN_source;
	GPIO_TypeDef *GPIO_MISO_PORT;
	uint16_t GPIO_MISO_PIN;
	uint16_t GPIO_MISO_PIN_source;
	GPIO_TypeDef *GPIO_SCK_PORT;
	uint16_t GPIO_SCK_PIN;
	uint16_t GPIO_SCK_PIN_source;
	GPIO_TypeDef*GPIO_CS_PORT;
	uint16_t GPIO_CS_PIN;
	SPI_TypeDef* spinum;
};
#endif
