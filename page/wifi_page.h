#ifndef __WIFI_PAGE_H_
#define __WIFI_PAGE_H_
struct front_struct;
typedef struct front_struct* front_handler;
struct image_struct;
typedef struct image_struct*image_handle;
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
void wifi_page_show(lcd24_handler lcd24x,
	front_handler frontx,front_handler frontx_cn,
	image_handle imagex);
extern image_handle image_sihuang_wifi_handle;
#endif
