//控制芯片
#ifndef __BOARD_H__
#define __BOARD_H__
#include "usart.h"
#include "timer.h"
#include "dht11.h"
#include "lcd24.h"
#include "front.h"
#include "image.h"
#include "esp32c3.h"
#include "weather.h"
#include "welcome_page.h"
#include "error_page.h"
#include "wifi.h"
#include "wifi_page.h"
#include "main_page.h"
#include "rtc.h"
#include "ui.h"
#include "flex_key.h"
struct my_i2c;
typedef struct my_i2c* i2c_handle;
void board_init(void);

extern USART_MY_handle USART_desc_1;
extern timer_handler timer_handler_1;
extern dth_handle dht11_1_handler;
extern lcd24_handler lcd241;
extern front_handler front_24X24;
extern front_handler front_32X32;
extern front_handler front_20X20;
extern image_handle image1;
extern esp32c3_handler esp32c3_1;
extern weather_handler weather_1;
extern rtc_handler rtc_handler_1;
extern flex_button_handler flex_handler_1;
#endif
