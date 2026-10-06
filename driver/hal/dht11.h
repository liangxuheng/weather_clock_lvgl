#ifndef __dht11_H_
#define __dht11_H_
#include <stdint.h>
#include <stdbool.h>
struct dht11_struct;
typedef struct dht11_struct* dth_handle;
void dht11_init(dth_handle dht11);
bool dht11_data_read(dth_handle dht11,uint8_t *buf,uint8_t length);
#endif
