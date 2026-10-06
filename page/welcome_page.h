#ifndef __WELCOME_PAGE_H_
#define __WELCOME_PAGE_H_
struct image_struct;
typedef struct image_struct*image_handle;
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
struct front_struct;
typedef struct front_struct* front_handler;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
extern image_handle image_heiqiyihu_welcome_handle;
void welcome_page_show(lcd24_handler lcd24x,image_handle imagex,
	front_handler frontx_cn,front_handler frontx_asc);
void welcome_page_init(void);
#endif
