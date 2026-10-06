#ifndef __MAIN_PAGE_H_
#define __MAIN_PAGE_H_
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
struct rtc_struct;
typedef struct rtc_struct* rtc_handler;
void main_page_show(lcd24_handler lcd24x,esp32c3_handler esp32x);
void main_page_redraw_wifi_ssid(lcd24_handler lcd24x,const char *ssid);
void main_page_redraw_time(lcd24_handler lcd24x,rtc_handler time);
void main_page_redraw_date(lcd24_handler lcd24x,rtc_handler date);
void main_page_redraw_inner_temperature( lcd24_handler lcd24x,float temperature);
void main_page_redraw_inner_humidity(lcd24_handler lcd24x,float humidity);
void main_page_redraw_outdoor_city(lcd24_handler lcd24x,const char *city);
void main_page_redraw_outdoor_temperature(lcd24_handler lcd24x,float temperature);
void main_page_redraw_outdoor_weather_icon(lcd24_handler lcd24x,const int code);
#endif
