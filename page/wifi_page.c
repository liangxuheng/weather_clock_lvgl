//#include "lcd24.h"
#include "ui.h"
//#include "front.h"
#include "front_desc.h"
#include "image.h"
#include "esp32c3.h"
#include "esp32c3_desc.h"
#include <string.h>
#define SSID "your_wifi_ssid"
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
#define RGB(r,g,b) ((r&0xF8)<<8|(g&0xFC)<<3|(b&0xF8)>>3)
void wifi_page_show(lcd24_handler lcd24x,
	front_handler frontx,front_handler frontx_cn,image_handle imagex)
{
//	lcd_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
//	lcd_show_image(lcd24x,30,15,imagex);
//	lcd_show_str(lcd24x,88,191,frontx,"WIFI",strlen("WIFI"),RGB(0,255,234),RGB(0,0,0));
		ui_set_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
		ui_set_show_image(lcd24x,30,15,imagex);
		ui_set_show_str(lcd24x,88,191,frontx,"WIFI",strlen("WIFI"),RGB(0,255,234),RGB(0,0,0));
	uint16_t startx=0;
	uint16_t len=strlen(SSID)*frontx->weight/2;
	if(len<240-1)
	{
		startx=(240+1-len)/2;
//		lcd_show_str(lcd24x,startx,231,frontx,(char*)SSID,strlen(SSID),RGB(255,255,255),RGB(0,0,0));
//		lcd_show_str(lcd24x,84,263,frontx_cn,"连接中",strlen("连接中"),RGB(148,198,255),RGB(0,0,0));
			ui_set_show_str(lcd24x,startx,231,frontx,(char*)SSID,strlen(SSID),RGB(255,255,255),RGB(0,0,0));
			ui_set_show_str(lcd24x,84,263,frontx_cn,"连接中",strlen("连接中"),RGB(148,198,255),RGB(0,0,0));
	}
}
