#ifndef __GPIO_H_
#define __GPIO_H_
#include <stdint.h>
struct flex_button;
typedef struct flex_button* flex_button_handler;
uint8_t usr_key_read(void *p);
void gpio_init(flex_button_handler flex_button_x);
#endif
