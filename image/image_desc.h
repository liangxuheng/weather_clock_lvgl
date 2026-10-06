#ifndef __IMAGE_DESC_H_
#define __IMAGE_DESC_H_
#include <stdint.h>
struct image_struct
{
	uint16_t height;
	uint16_t weight;
	const uint8_t *pdata_arr;
};
extern const unsigned char gImage_image_data[];
extern const unsigned char gImage_image_heiqiyihu[];
extern const unsigned char gImage_image_zuozhu[];
extern const unsigned char gImage_image_sihuang[];
extern const unsigned char gImage_icon_wifi[];
extern const unsigned char gImage_icon_wenduji[];
extern const unsigned char gImage_icon_leizhenyu[];
extern const unsigned char gImage_icon_na[];
extern const unsigned char gImage_icon_yintian[];
extern const unsigned char gImage_icon_yueliang[];
extern const unsigned char gImage_icon_zhongxue[];
extern const unsigned char gImage_icon_zhongyu[];
extern const unsigned char gImage_icon_duoyun[];
#endif
