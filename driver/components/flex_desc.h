#ifndef __FLEX_DESC_KEY_H_
#define __FLEX_DESC_KEY_H_
#include <stdint.h>
#include "stm32f4xx.h"
typedef struct{
	GPIO_TypeDef * GPIO_PORT;//端口
	uint16_t GPIO_PIN;//引脚
	BitAction on;//导通电平
	BitAction off;//断开电平
}GPIO_T;
typedef GPIO_T * GPIO_Handler;
typedef void(*flex_button_callback)(void*);
struct flex_button{
	//按键是通过链表连起来的
	struct flex_button* next;
	//读取引脚电平的函数指针
	uint8_t (*flex_usr_read)(void*);
	//按键事件触发时的回调函数
	flex_button_callback cb;
	//按键按下时的电平
	uint8_t press_level :1;
	//按键上次触发的事件
	uint8_t event :4;
	//按键的状态
	uint8_t status :3;
	//按键号
	uint8_t id;
	//按键扫描的时间
	uint16_t scan_cnt;
	//点击的次数
	uint16_t click_cnt;
	//双击或者多击事件之间的最大间隔事件
	uint16_t max_multiple_clicks_interval;
	//按键消抖时间
	uint16_t debounce_tick;
	//短按阈值
	uint16_t short_press_tick;
	//长按阈值
	uint16_t long_press_tick;
	//长保持阈值
	uint16_t long_hold_tick;
	//对应的GPIO引脚信息
	GPIO_Handler gpio;
};
enum FLEX_BTN_STAGE
{
    FLEX_BTN_STAGE_DEFAULT = 0,
    FLEX_BTN_STAGE_DOWN    = 1,
    FLEX_BTN_STAGE_MULTIPLE_CLICK = 2
};

#endif
