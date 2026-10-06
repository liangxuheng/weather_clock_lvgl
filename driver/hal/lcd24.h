#ifndef __LCD24_H_
#define __LCD24_H_
#include <stdint.h>
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
#define RGB(r,g,b) ((r&0xF8)<<8|(g&0xFC)<<3|(b&0xF8)>>3)
void lcd_init(lcd24_handler lcd24_x);
void lcd_fill_colour(lcd24_handler lcd24_x,uint16_t x1\
	,uint16_t x2,uint16_t y1,uint16_t y2,uint16_t colour );
void lcd_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs,uint16_t colour_bg);
void lcd_show_image(lcd24_handler lcd24x,uint16_t x,uint16_t y,image_handle imagex);
uint16_t lcd_get_width(void);
uint16_t lcd_get_height(void);
void lcd_show_buffer(lcd24_handler lcd24_x, uint16_t x1, uint16_t x2\
	, uint16_t y1, uint16_t y2, const uint16_t *buf);
#endif
