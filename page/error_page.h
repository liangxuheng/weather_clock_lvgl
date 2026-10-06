#ifndef __ERROR_PAGE_H_
#define __ERROR_PAGE_H_
struct image_struct;
typedef struct image_struct*image_handle;
struct lcd24;
typedef struct lcd24* lcd24_handler;
struct front_struct;
typedef struct front_struct* front_handler;
void error_page_show(lcd24_handler lcd24x,
	const char* msg,image_handle imagex,front_handler frontx);
extern image_handle image_zuozhu_error_handle;
#endif
