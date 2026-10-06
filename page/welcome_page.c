//#include "lcd24.h"
//#include "systick.h"
//#include "front.h"
//#include "image.h"
#include "usart.h"
#include "timer.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
#define RGB(r,g,b) ((r&0xF8)<<8|(g&0xFC)<<3|(b&0xF8)>>3)
extern timer_handler timer_handler_1;
//extern lcd24_handler lcd241;
//extern USART_MY_handle USART_desc_1;
//extern front_handler front_24X24;
//extern front_handler front_32X32;
void welcome_page_show(lcd24_handler lcd24x,image_handle imagex,front_handler frontx_cn,front_handler frontx_asc)
{
//	lcd_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
//	lcd_show_image(lcd24x,30,10,imagex);
//	lcd_show_str(lcd24x,56,205,frontx_cn,"黑崎一护",strlen("黑崎一护"),RGB(237,128,147),RGB(0,0,0));
//	lcd_show_str(lcd24x,56,233,frontx_cn,"天气时钟",strlen("天气时钟"),RGB(86,165,255),RGB(0,0,0));
//	lcd_show_str(lcd24x,60,285,frontx_asc,"loading...",strlen("loading..."),RGB(255,255,255),RGB(0,0,0));
		ui_set_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
		ui_set_show_image(lcd24x,30,10,imagex);
		ui_set_show_str(lcd24x,56,205,frontx_cn,"黑崎一护",strlen("黑崎一护"),RGB(237,128,147),RGB(0,0,0));
		ui_set_show_str(lcd24x,56,233,frontx_cn,"天气时钟",strlen("天气时钟"),RGB(86,165,255),RGB(0,0,0));
		ui_set_show_str(lcd24x,60,285,frontx_asc,"loading...",strlen("loading..."),RGB(255,255,255),RGB(0,0,0));
}

void welcome_page_init(void)
{

	//systick_my_init();
//	USART_init(usartx);
	m_time_init(timer_handler_1);
	//printf("Usart_init\r\n\r\n");
	//lcd_init(lcd24x);
	printf("Build: %s %s\r\n\r\n",__DATE__,__TIME__);
}
