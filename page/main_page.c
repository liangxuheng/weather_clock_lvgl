//#include "front.h"
//#include "front_desc.h"
#include "esp32c3.h"
#include "image.h"
//#include "lcd24.h"
#include "ui.h"
#include "rtc.h"
#include "rtc_desc.h"
#include "weather.h"
#include "dht11.h"
#include <string.h>
#include <stdio.h>
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
#define RGB(r,g,b) ((r&0xF8)<<8|(g&0xFC)<<3|(b&0xF8)>>3)
//#define WEATHER_URL "https://api.seniverse.com/v3/weather/now.json?key=YOUR_API_KEY&location=YOUR_LOCATION_ID&language=zh-Hans&unit=c"
//extern weather_handler weather_1;
//extern dth_handle dht11_1_handler;
static const uint16_t color_bg_top = RGB(248, 248, 248);
static const uint16_t color_bg_inner = RGB(136, 217, 234);
static const uint16_t color_bg_outdoor = RGB(254, 135, 75);
extern image_handle image_duoyun;
extern image_handle image_zhongyu;
extern image_handle image_zhongxue;
extern image_handle image_yueliang;
extern image_handle image_yintian;
extern image_handle image_na;
extern image_handle image_leizhenyu;
//extern rtc_handler rtc_handler_1;
extern image_handle image_wifi_icon;
extern image_handle image_wenduji_icon;
extern front_handler front_54X54;
extern front_handler front16X16;
extern front_handler front76X76;
extern front_handler front_20X20;
extern front_handler front_32X32;
extern front_handler front_54X54;
extern front_handler front_64X64;
extern image_handle image_qing_icon;
//void main_page_redraw_wifi_ssid(lcd24_handler lcd24x,const char *ssid);
//void main_page_redraw_time(lcd24_handler lcd24x,rtc_handler time);
//void main_page_redraw_date(lcd24_handler lcd24x,rtc_handler date);
//void main_page_redraw_inner_temperature( lcd24_handler lcd24x,float temperature);
//void main_page_redraw_inner_humidity(lcd24_handler lcd24x,float humidity);
//void main_page_redraw_outdoor_city(lcd24_handler lcd24x,const char *city);
//void main_page_redraw_outdoor_temperature(lcd24_handler lcd24x,float temperature);
//void main_page_redraw_outdoor_weather_icon(lcd24_handler lcd24x,const int code);
void main_page_show(lcd24_handler lcd24x,esp32c3_handler esp32x)
{
	//lcd_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
	ui_set_fill_colour(lcd24x,0,240-1,0,320-1,RGB(0,0,0));
	//最顶上的页面
	do{
//		lcd_fill_colour(lcd24x,15,224,15,154,color_bg_top );
//		lcd_show_image(lcd24x,23,20,image_wifi_icon);
		ui_set_fill_colour(lcd24x,15,224,15,154,color_bg_top);
		ui_set_show_image(lcd24x,23,20,image_wifi_icon);
		//const char *ssid=esp_get_wifi_ssid(esp32x);		
//		uint32_t len=strlen(ssid)*front16X16->weight/2;
//		uint8_t startx=50;
////		if(len<216)
////		{
////			if(216-len>50)
////			{
////				startx=216-len;
////			}
////		}
//		if(len<216-50)
//		{
//			startx=216-len;
//		}
//		lcd_show_str(lcd24x,startx,23,front16X16,(char*)ssid,strlen(ssid),RGB(143,143,143)
//		,color_bg_top );
		//main_page_redraw_wifi_ssid(lcd24x,ssid);
//		lcd_show_str(lcd24x,25,42,front76X76,"--:--",strlen("--:--"),RGB(0,0,0)
//		,color_bg_top );
		ui_set_show_str(lcd24x,25,42,front76X76,"--:--",strlen("--:--"),RGB(0,0,0)
		,color_bg_top );
		//main_page_redraw_time(lcd24x,rtc_handler_1);
//		lcd_show_str(lcd24x,35,121,front_20X20,"----/--/-- 星期--",strlen("----/--/-- 星期--")
//		,RGB(143,143,143),color_bg_top );
			ui_set_show_str(lcd24x,35,121,front_20X20,"----/--/-- 星期--",strlen("----/--/-- 星期--")
		,RGB(143,143,143),color_bg_top );
		//main_page_redraw_date(lcd24x,rtc_handler_1);
	}while(0);
	
	//室内
	do{
		//const uint16_t colour_bg=RGB(136,217,234);
//		lcd_fill_colour(lcd24x,15,114,165,304,color_bg_inner);
//		lcd_show_str(lcd24x,19,170,front16X16,"室内环境",strlen("室内环境"),RGB(0,0,0),color_bg_inner);
//		lcd_show_str(lcd24x,86,191,front_32X32,"C",strlen("C"),RGB(0,0,0),color_bg_inner);
//		lcd_show_str(lcd24x,91,262,front_32X32,"%",strlen("%"),RGB(0,0,0),color_bg_inner);
			ui_set_fill_colour(lcd24x,15,114,165,304,color_bg_inner);
			ui_set_show_str(lcd24x,19,170,front16X16,"室内环境",strlen("室内环境"),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,86,191,front_32X32,"C",strlen("C"),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,91,262,front_32X32,"%",strlen("%"),RGB(0,0,0),color_bg_inner);
//		uint8_t data[5]={0};
//		dht11_data_read(dht11_1_handler,data,sizeof(data)/sizeof(uint8_t));
		//{
			//printf("dht11 read success\r\n\r\n,&s,%s",__FILE__,__FUNCTION__);
		//}
		//main_page_redraw_inner_temperature(lcd24x,(float)data[2]);
//		lcd_show_str(lcd24x,30,192,front_54X54,"--",strlen("--"),RGB(0,0,0),color_bg_inner);
//		lcd_show_str(lcd24x,28,239,front_64X64,"--",strlen("--"),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,30,192,front_54X54,"--",strlen("--"),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,28,239,front_64X64,"--",strlen("--"),RGB(0,0,0),color_bg_inner);
		//main_page_redraw_inner_humidity(lcd24x,(float)data[0]);
	}while(0);
	
	//室外
	do{
		//const uint16_t colour_bg=RGB(254,135,75);
//		lcd_fill_colour(lcd24x,125,224,165,304,color_bg_outdoor);
			ui_set_fill_colour(lcd24x,125,224,165,304,color_bg_outdoor);
//		char *city=NULL;
//		int code=-1;
//		float temperature=0;
//		if(esp_http_get(esp32x,WEATHER_URL))
//		{
//			if(get_weather(esp32x,weather_1))
//			{
//				code=get_weather_code(weather_1);
//				city=(char*)get_weather_city(weather_1);
//				temperature=get_weather_temperature(weather_1);
//			}
//		}
		//main_page_redraw_outdoor_city(lcd24x,city);
		//lcd_show_str(lcd24x,127,170,front_20X20,"----",strlen("----"),RGB(0,0,0),color_bg_outdoor);
		//lcd_show_str(lcd24x,140,186,front_54X54,"--",strlen("--"),RGB(0,0,0),color_bg_outdoor);
		//main_page_redraw_outdoor_temperature(lcd24x,temperature);
		//lcd_show_image(lcd24x,139,239,image_wenduji_icon);
			ui_set_show_image(lcd24x,139,239,image_wenduji_icon);
		//lcd_show_image(lcd24x,166,240,image_qing_icon);
		//main_page_redraw_outdoor_weather_icon(lcd24x,code);
	}while(0);
}

void main_page_redraw_wifi_ssid(lcd24_handler lcd24x,const char *ssid)
{
    char str[21]={0};
		//if(ssid){
    snprintf(str, sizeof(str), "%20s", ssid);
    //st7789_write_string(50, 23, str, mkcolor(143, 143, 143), color_bg_time, &font16_maple);
		//lcd_show_str(lcd24x,50,23,front16X16,str,strlen(str),RGB(143,143,143),color_bg_top);
		ui_set_show_str(lcd24x,50,23,front16X16,str,strlen(str),RGB(143,143,143),color_bg_top);
		//			return;
		//}
}

void main_page_redraw_time(lcd24_handler lcd24x,rtc_handler time)
{
			char str[6]={0};
//    char comma = (time->second % 2 == 0) ? ':' : ' ';
			memset(time,0,sizeof(struct rtc_struct));
			rtc_get_time(time);
			char comma=(time->second%2)?':':' ';
		snprintf(str, sizeof(str), "%02u%c%02u", time->hour,comma,time->minute);
    //st7789_write_string(25, 42, str, mkcolor(0, 0, 0), color_bg_time, &font76_maple_extrabold);
		//lcd_show_str(lcd24x,25,42,front76X76,str,strlen(str),RGB(0,0,0),color_bg_top);
			ui_set_show_str(lcd24x,25,42,front76X76,str,strlen(str),RGB(0,0,0),color_bg_top);
}

void main_page_redraw_date(lcd24_handler lcd24x,rtc_handler date)
{
    char str[21]={0};
		memset(date,0,sizeof(struct rtc_struct));
		rtc_get_time(date);
		//printf("%d\r\n,%s,%s",date->weekday,__FILE__,__FUNCTION__);
    snprintf(str, sizeof(str), "%04u/%02u/%02u 星期%s", date->year, date->month, date->day,
        date->weekday == 1 ? "一" :
        date->weekday == 2 ? "二" :
        date->weekday == 3 ? "三" :
        date->weekday == 4 ? "四" :
        date->weekday == 5 ? "五" :
        date->weekday == 6 ? "六" :
				date->weekday == 7 ? "天":"X");
		//printf("%s\r\n,%s,%s",str,__FILE__,__FUNCTION__);
    //st7789_write_string(35, 121, str, mkcolor(143, 143, 143), color_bg_time, &font20_maple_bold);
	//lcd_show_str(lcd24x,35,121,front_20X20,str,strlen(str),RGB(143, 143, 143),color_bg_top);
		ui_set_show_str(lcd24x,35,121,front_20X20,str,strlen(str),RGB(143, 143, 143),color_bg_top);
}


void main_page_redraw_inner_temperature( lcd24_handler lcd24x,float temperature)
{
    char str[3] = {'-', '-'};
    if (temperature > -10.0f && temperature <= 100.0f)
        snprintf(str, sizeof(str), "%2.0f", temperature);
    //st7789_write_string(30, 192, str, mkcolor(0, 0, 0), color_bg_inner, &font54_maple_semibold);
		//lcd_show_str(lcd24x,30,192,front_54X54,str,strlen(str),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,30,192,front_54X54,str,strlen(str),RGB(0,0,0),color_bg_inner);
}

void main_page_redraw_inner_humidity(lcd24_handler lcd24x,float humidity)
{
    char str[3]={'-','-'};
    if (humidity > 0.0f && humidity <= 99.99f)
        snprintf(str, sizeof(str), "%2.0f", humidity);
    //st7789_write_string(25, 239, str, mkcolor(0, 0, 0), color_bg_inner, &font64_maple_extrabold);
		//lcd_show_str(lcd24x,25,239,front_64X64,str,strlen(str),RGB(0,0,0),color_bg_inner);
			ui_set_show_str(lcd24x,25,239,front_64X64,str,strlen(str),RGB(0,0,0),color_bg_inner);
}

void main_page_redraw_outdoor_city(lcd24_handler lcd24x,const char *city)
{
    char str[9]={0};
    snprintf(str, sizeof(str), "%s", city);
    //st7789_write_string(127, 170, str, mkcolor(0, 0, 0), color_bg_outdoor, &font24_maple_semibold);
		//lcd_show_str(lcd24x,127,170,front_20X20,str,strlen(str),RGB(0,0,0),color_bg_outdoor);
			ui_set_show_str(lcd24x,127,170,front_20X20,str,strlen(str),RGB(0,0,0),color_bg_outdoor);
}

void main_page_redraw_outdoor_temperature(lcd24_handler lcd24x,float temperature)
{
    char str[3] = {'-', '-'};
    if (temperature > -10.0f && temperature <= 100.0f)
        snprintf(str, sizeof(str), "%2.0f", temperature);
    //st7789_write_string(135, 190, str, mkcolor(0, 0, 0), color_bg_outdoor, &font54_maple_bold);
		//lcd_show_str(lcd24x,135,190,front_54X54,str,strlen(str),RGB(0,0,0),color_bg_outdoor);
			ui_set_show_str(lcd24x,135,190,front_54X54,str,strlen(str),RGB(0,0,0),color_bg_outdoor);
}

void main_page_redraw_outdoor_weather_icon(lcd24_handler lcd24x,const int code)
{
    image_handle icon;
    if (code == 0 || code == 2 || code == 38){icon=image_qing_icon;}
        //icon = &icon_qing;
    else if (code == 1 || code == 3){icon=image_yueliang;}
        //icon = &icon_yueliang;
    else if (code == 4 || code == 9){icon=image_yintian;}
        //icon = &icon_yintian;
    else if (code == 5 || code == 6 || code == 7 || code == 8)
		{icon=image_duoyun;}   //icon = &icon_duoyun;
    else if (code == 10 || code == 13 || code == 14 || code == 15 || code == 16 || code == 17 || code == 18 || code == 19)
		{icon=image_zhongyu;}   //icon = &icon_zhongyu;
    else if (code == 11 || code == 12)
    {icon=image_leizhenyu;}    //icon = &icon_leizhenyu;
    else if (code == 20 || code == 21 || code == 22 || code == 23 || code == 24 || code == 25)
    {icon=image_zhongxue;}    //icon = &icon_zhongxue;
    else
    {icon=image_na;}    //icon = &icon_na;
    //st7789_draw_image(166, 240, icon);
		//lcd_show_image(lcd24x,166,240,icon);
			ui_set_show_image(lcd24x,166,240,icon);
}
