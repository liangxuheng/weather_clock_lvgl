//#include "front.h"
#include "front_desc.h"
//#include "image.h"
//#include "lcd24.h"
#include "ui.h"
#include <string.h>
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
#define RGB(r,g,b) ((r&0xF8)<<8|(g&0xFC)<<3|(b&0xF8)>>3)
void error_page_show(lcd24_handler lcd24x,const char* msg,image_handle imagex,front_handler frontx)
{
//	lcd_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
//	lcd_show_image(lcd24x,40,37,imagex);
		ui_set_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
		ui_set_show_image(lcd24x,40,37,imagex);
	uint16_t startx=0;
	uint16_t len=strlen(msg)*frontx->weight/2;
	if(len<240)
	{
		startx=(240-len+1)/2;
//		lcd_show_str(lcd24x,startx,245,frontx,(char*)msg,strlen(msg),RGB(255,255,0),RGB(0,0,0));
			ui_set_show_str(lcd24x,startx,245,frontx,(char*)msg,strlen(msg),RGB(255,255,0),RGB(0,0,0));
	}
}
