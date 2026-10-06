//主要是做声明
#ifndef __LED_H__
#define __LED_H__
#include <stdbool.h>
//只做声明(不对外暴露结构体的细节)
struct led_desc;
typedef struct led_desc* led_desc_t;
//由于没有结构体的实现，所以只能用指针
void led_init(led_desc_t ledx);
void led_on(led_desc_t ledx);
void led_off(led_desc_t ledx);
#endif
