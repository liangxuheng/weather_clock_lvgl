/*
 * lv_font_maple.c
 * 将原 1bpp 点阵字模（Maple Mono NF CN）包装成 LVGL 自定义字体
 * 不修改原有字模数据，只做适配层
 */
#include "front.h"
#include "front_desc.h"
#include "lvgl/lvgl.h"
#include <string.h>
#include <stdint.h>

/* board.c 里定义的字体句柄 */
extern front_handler front_24X24;
extern front_handler front_32X32;
extern front_handler front_20X20;
extern front_handler front16X16;
extern front_handler front76X76;
extern front_handler front_54X54;
extern front_handler front_64X64;

/* 把 front_struct 和 lv_font_t 绑定 */
typedef struct {
    front_handler front;
    lv_font_t     lv_font;
} maple_font_dsc_t;

static maple_font_dsc_t font_dsc_16;
static maple_font_dsc_t font_dsc_20;
static maple_font_dsc_t font_dsc_24;
static maple_font_dsc_t font_dsc_32;
static maple_font_dsc_t font_dsc_54;
static maple_font_dsc_t font_dsc_64;
static maple_font_dsc_t font_dsc_76;

/* 供 lcd24.c 的 front_to_lvgl_font 取用 */
const lv_font_t *lv_font_maple_get(front_handler frontx);

/* ---------------------------------------------------------------
 * Unicode codepoint -> UTF-8 字节串
 * 返回字节数 1~4，结果写入 out_buf（至少 4 字节）
 * ------------------------------------------------------------- */
static int unicode_to_utf8(uint32_t cp, uint8_t *out_buf)
{
    if (cp < 0x80) {
        out_buf[0] = (uint8_t)cp;
        return 1;
    } else if (cp < 0x800) {
        out_buf[0] = (uint8_t)(0xC0 | (cp >> 6));
        out_buf[1] = (uint8_t)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp < 0x10000) {              /* 中文在此范围 */
        out_buf[0] = (uint8_t)(0xE0 | (cp >> 12));
        out_buf[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        out_buf[2] = (uint8_t)(0x80 | (cp & 0x3F));
        return 3;
    } else {
        out_buf[0] = (uint8_t)(0xF0 | (cp >> 18));
        out_buf[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
        out_buf[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        out_buf[3] = (uint8_t)(0x80 | (cp & 0x3F));
        return 4;
    }
}

/* ---------------------------------------------------------------
 * get_glyph_dsc：返回字形尺寸
 * ------------------------------------------------------------- */
static bool maple_get_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *dsc,
                                uint32_t letter, uint32_t letter_next)
{
    front_handler front = (front_handler)font->dsc;
    if (!front) return false;

    /* ASCII 可打印字符 0x20~0x7E */
    if (letter >= 0x20 && letter <= 0x7E) {
        if (front->map) {
            const char *p = strchr((const char *)front->map, (char)letter);
            if (!p) return false;
        }
				//`box_w` ：字形点阵实际宽度。ASCII 是半宽，所以`weight/2`
				//`box_h` ：字形点阵高度 = 字高
				//`ofs_x/ofs_y` ：点阵相对笔尖的偏移。你的点阵本身就是顶对齐满框，没有偏移，填 0
				//`adv_w` ：advance width，画完这个字后画笔右移多少像素。=半宽，即字与字紧挨
				//`bpp` ：每像素位数，点阵黑白字模填 1（告诉 LVGL 按 1bpp 格式读位图）
				//`is_placeholder` ：0 表示真实字形（LVGL 里为 1 时表示"缺字占位符"，仍会画个框）
        dsc->box_w  = front->weight / 2;   /* ASCII 半宽 */
        dsc->box_h  = front->height;
        dsc->ofs_x  = 0;
        dsc->ofs_y  = 0;
				//从当前字偏移到下一个字的像素距离(是指屏幕上的距离，就是字与字之间的距离)
        dsc->adv_w  = front->weight / 2;
        dsc->bpp    = 1;
        dsc->is_placeholder = 0;
        return true;
    }

    /* 中文：Unicode -> UTF-8 -> 匹配 chinese[].str */
    if (front->chinese && front->len > 0) {
        uint8_t utf8[5] = {0};
        int n = unicode_to_utf8(letter, utf8);
        utf8[n] = '\0';
        for (uint32_t i = 0; i < front->len; i++) {
            if (strcmp((const char *)utf8, front->chinese[i].str) == 0) {
                dsc->box_w  = front->weight;     /* 中文全宽 */
                dsc->box_h  = front->height;
                dsc->ofs_x  = 0;
                dsc->ofs_y  = 0;
                dsc->adv_w  = front->weight;
                dsc->bpp    = 1;
                dsc->is_placeholder = 0;
                return true;
            }
        }
    }
    return false;
}

/* ---------------------------------------------------------------
 * 字模重排缓冲
 * 原字库按行存储、每行末尾补齐到整字节；
 * LVGL 要求行与行紧密打包（不留补齐位）。
 * LVGL 取 bitmap 后立即同步绘制（ui_task 单线程），共用一个缓冲即可。
 * 最大字号 76x76，紧凑格式最大 760 字节。
 * ------------------------------------------------------------- */
#define MAPLE_MAX_W     76
#define MAPLE_MAX_H     76
static uint8_t glyph_pack_buf[((MAPLE_MAX_W + 7) / 8) * MAPLE_MAX_H];

/* 把"逐行补整齐字节"的原字模重排为 LVGL 的紧密 1bpp 格式 */
static const uint8_t *maple_pack_bitmap(const uint8_t *src,
                                        uint16_t visual_w, uint16_t height)
{
    if (!src) return NULL;
    uint16_t src_row_bytes = (uint16_t)((visual_w + 7) / 8);
    uint32_t dst_size = ((uint32_t)visual_w * height + 7) / 8;
    memset(glyph_pack_buf, 0, dst_size);
    for (uint16_t r = 0; r < height; r++) {
        for (uint16_t c = 0; c < visual_w; c++) {
            uint8_t bit = (src[r * src_row_bytes + (c >> 3)] >> (7 - (c & 7))) & 0x01;
            if (bit) {
                uint32_t dst_bit = (uint32_t)r * visual_w + c;
                glyph_pack_buf[dst_bit >> 3] |= (uint8_t)(0x80 >> (dst_bit & 7));
            }
        }
    }
    return glyph_pack_buf;
}

/* ---------------------------------------------------------------
 * get_glyph_bitmap：返回重排后的字形 bitmap 指针
 * ------------------------------------------------------------- */
static const uint8_t *maple_get_glyph_bitmap(const lv_font_t *font, uint32_t letter)
{
    front_handler front = (front_handler)font->dsc;
    if (!front) return NULL;

    if (letter >= 0x20 && letter <= 0x7E) {
        uint32_t index;
        if (front->map) {
            const char *p = strchr((const char *)front->map, (char)letter);
            if (!p) return NULL;
            index = (uint32_t)(p - (const char *)front->map);
        } else {
            index = letter - 0x20;
        }
        uint32_t bytes_per_char = front->height *
                                  (((front->weight / 2) + 7) / 8);
        return maple_pack_bitmap(front->data_arr + index * bytes_per_char,
                                 front->weight / 2, front->height);
    }

    if (front->chinese && front->len > 0) {
        uint8_t utf8[5] = {0};
        int n = unicode_to_utf8(letter, utf8);
        utf8[n] = '\0';
        for (uint32_t i = 0; i < front->len; i++) {
            if (strcmp((const char *)utf8, front->chinese[i].str) == 0) {
                return maple_pack_bitmap(front->chinese[i].data_mode,
                                         front->weight, front->height);
            }
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * 初始化单个自定义字体
 * ------------------------------------------------------------- */
static void maple_font_init(maple_font_dsc_t *dsc, front_handler front)
{
		//绑定结构体里存一份原字模
		//最核心的两行 ，把我们写的两个回调函数地址注册给 LVGL。以后 LVGL 画这个字体的字，就会回头调它们
		//把原字模指针存进`lv_font.dsc` ——LVGL 不解释这个字段，回调时原样通过`font->dsc` 传回（第 71、147 行就是这么取回来的）
		//行高=字高
		//`base_line=0` 。LVGL 字形顶点公式`gpos.y = pos.y + (line_height - base_line) - box_h` ，代入 box_h=line_height、base_line=0 得`gpos.y = pos.y` ，字形顶部正好贴 label 顶部。之前设成字高导致整字上移一个字高被裁掉（黑点根因 2）
		//关闭子像素渲染（单色屏无意义，还要求位图横向放大 3 倍）
		//下划线位置/粗细都 0（点阵字不画下划线，且 label 也没开下划线样式）
		//`fallback=NULL` ，本字体找不到的字不再去别的字体找，直接不显示
    dsc->front = front;
    dsc->lv_font.get_glyph_dsc    = maple_get_glyph_dsc;
    dsc->lv_font.get_glyph_bitmap = maple_get_glyph_bitmap;
    dsc->lv_font.dsc              = front;
    dsc->lv_font.line_height      = front->height;
    /* base_line=0：字形框高等于字高，绘制顶点恰好落在 label 顶部 pos.y */
    dsc->lv_font.base_line        = 0;
    dsc->lv_font.subpx            = LV_FONT_SUBPX_NONE;
    dsc->lv_font.underline_position    = 0;
    dsc->lv_font.underline_thickness   = 0;
    dsc->lv_font.fallback         = NULL;
}

/* ---------------------------------------------------------------
 * 外部入口：在 lv_init() 之后调用一次
 * ------------------------------------------------------------- */
void lv_font_maple_init(void)
{
    maple_font_init(&font_dsc_16, front16X16);
    maple_font_init(&font_dsc_20, front_20X20);
    maple_font_init(&font_dsc_24, front_24X24);
    maple_font_init(&font_dsc_32, front_32X32);
    maple_font_init(&font_dsc_54, front_54X54);
    maple_font_init(&font_dsc_64, front_64X64);
    maple_font_init(&font_dsc_76, front76X76);
}

/* ---------------------------------------------------------------
 * front_to_lvgl_font 辅助：根据 front 的 height 取对应字体
 * ------------------------------------------------------------- */
const lv_font_t *lv_font_maple_get(front_handler frontx)
{
    if (!frontx) return &font_dsc_24.lv_font;
    switch (frontx->height) {
        case 16: return &font_dsc_16.lv_font;
        case 20: return &font_dsc_20.lv_font;
        case 24: return &font_dsc_24.lv_font;
        case 32: return &font_dsc_32.lv_font;
        case 54: return &font_dsc_54.lv_font;
        case 64: return &font_dsc_64.lv_font;
        case 76: return &font_dsc_76.lv_font;
        default: return &font_dsc_24.lv_font;
    }
}
