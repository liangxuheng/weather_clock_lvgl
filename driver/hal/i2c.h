#ifndef __I2C_M_H_
#define __I2C_M_H_
#include <stdint.h>
#include <stdbool.h>
struct my_i2c;
typedef struct my_i2c* i2c_handle;
void m_i2c_init(i2c_handle i2cx);
bool i2c_read(i2c_handle i2cx,uint8_t address_slave, uint8_t address ,uint8_t *buf,uint32_t length);
bool i2c_write(i2c_handle i2cx,uint8_t address_slave, uint8_t address ,uint8_t *buf,uint32_t length);
#endif
