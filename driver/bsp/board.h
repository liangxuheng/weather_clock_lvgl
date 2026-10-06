//控制芯片
#ifndef __BOARD_H__
#define __BOARD_H__
/* 前向声明各模块句柄类型，不再 include 任何模块头；
   访问结构体成员的文件需自行 include 对应模块头（参照 balance_car board.h 模式） */
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
struct timer_struct;
typedef struct timer_struct* timer_handler;
struct dht11_struct;
typedef struct dht11_struct* dth_handle;
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct* image_handle;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
struct weather;
typedef struct weather* weather_handler;
struct rtc_struct;
typedef struct rtc_struct* rtc_handler;
struct flex_button;
typedef struct flex_button* flex_button_handler;
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
