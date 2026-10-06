#ifndef __SPI_M_H_
#define __SPI_M_H_
#include <stdint.h>
struct spi_Struct;
typedef struct spi_Struct* spi_handler;
void  m_spi_init(spi_handler spix);
void spi_send_data(spi_handler spix,const uint8_t *data);
#endif
