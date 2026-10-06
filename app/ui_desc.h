#ifndef __UI_DESC_M_H
#define __UI_DESC_M_H
#pragma anon_unions
#include "front.h"
#include "image.h"
#include "lcd24.h"
#include "lcd24_desc.h"
//void lcd_init(lcd24_handler lcd24_x);
//void lcd_fill_colour(lcd24_handler lcd24_x,uint16_t x1\
//	,uint16_t x2,uint16_t y1,uint16_t y2,uint16_t colour );
//void lcd_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
//	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs,uint16_t colour_bg);
//void lcd_show_image(lcd24_handler lcd24x,uint16_t x,uint16_t y,image_handle imagex);
typedef enum{
	UI_FILL_COLOUR,
	UI_SHOW_STR,
	UI_SHOW_IMAGE
}ui_message_t;
struct ui_message
{
	ui_message_t action;
	union{
		struct {
			lcd24_handler lcd24x;
			uint16_t x1;
			uint16_t x2;
			uint16_t y1;
			uint16_t y2;
			uint16_t colour;
		}ui_fill_colour_structer;
		struct {
			lcd24_handler lcd24y;
			uint16_t x;
			uint16_t y;
			front_handler frontx;
			char*str;
			uint16_t legth;
			uint16_t colour_asc;
			uint16_t colour_bg;
		}ui_show_str_structer;
		struct{
		lcd24_handler lcd24z;
			uint16_t _x;
			uint16_t _y;
			image_handle imagex;
		} ui_show_image_structer;
	};
};
#endif
