#ifndef __FRONT_DESC_H_
#define __FRONT_DESC_H_
#include <stdint.h>
struct Chinese
{
	const char* str;//对应的汉字
	const uint8_t* data_mode;//指向该汉字的字符数组
};
typedef const struct Chinese* Chinese_handler;
struct front_struct
{
	uint16_t height;
	uint16_t weight;
	const uint8_t* data_arr;
	Chinese_handler chinese;
	uint32_t len;//字模数组的长度
	const uint8_t*map;//是否存了不是连续的acs字模
};
extern const uint8_t front_data_16[];
extern const uint8_t front_data_32[];
extern const uint8_t front_data_24[];
extern const struct Chinese front_data32X32_CH[];
extern const uint8_t front_data_20[];
extern const uint8_t ascii_model_16[];
extern const unsigned char ascii_model_76[];
extern const struct Chinese chinese_fonts_20[];
extern const struct Chinese chinese_fonts_ext_16[];
extern const uint8_t ascii_model_54[];
extern const uint8_t ascii_model_64[];
extern const unsigned char gImage_icon_qing[];
#endif
