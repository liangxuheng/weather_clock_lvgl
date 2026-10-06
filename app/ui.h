#ifndef __UI_M_H
#define __UI_M_H
#include <stdint.h>
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
struct ui_message;
typedef struct ui_message* ui_message_handler;
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
void ui_init(lcd24_handler lcd24x,USART_MY_handle usartx);
void ui_set_fill_colour(lcd24_handler lcd24x,uint16_t x1,uint16_t x2,uint16_t y1\
	,uint16_t y2,uint16_t colour);
void ui_set_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs\
		,uint16_t colour_bg);
	void ui_set_show_image(lcd24_handler lcd24x,uint16_t x\
		,uint16_t y,image_handle imagex);
#endif
