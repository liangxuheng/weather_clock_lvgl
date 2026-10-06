/**
 ******************************************************************************
 * @file    flex_key.c
 * @brief   柔性按键驱动：状态机消抖，支持短按/长按/连发
 ******************************************************************************
 */

#include "flex_desc.h"
#include "flex_key.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "gpio.h"
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
SemaphoreHandle_t wait_for_reconnect=NULL;
//extern SemaphoreHandle_t wait_for_delete;
TaskHandle_t task_flex_button_handle=NULL; 

#define MAX_MULTIPLE_CLICKS_INTERVAL \
(FlEX_KEY_MS_TO_SCANTICK(300))

//记录每个按键的逻辑电平
static uint32_t logic_elec_level=0;

#define ISPRESS(x) (logic_elec_level&(0x1<<(x)))

static flex_button_handler flex_button_head=NULL;

//记录每次按键的真实物理电平的位图
static uint32_t real_elec_level=0;

#define SET_EVENT_ANG_CB(bt,evt) \
do{\
	bt->event=evt;	\
	if(bt->cb)\
	{\
		bt->cb(bt);\
	}\
}while(0)

//记录注册按键的个数
static uint8_t register_button_cnt=0;

static int8_t flex_button_register(flex_button_handler \
	button,flex_button_callback cb)
{
		if(button==NULL||
		register_button_cnt>=sizeof(real_elec_level)*8)
		{
			//按键的句柄为空或者注册按键的个数大于位图的限制
				return -1;
		}
		flex_button_handler temp=flex_button_head;
		while(temp)
		{
			if(temp==button)
			{
				//已经注册过了
				return -1;
			}
			temp=temp->next;
		}
		button->cb=cb;
		real_elec_level|=(button->press_level
		<<(register_button_cnt));
		register_button_cnt++;
		button->next=flex_button_head;
		flex_button_head=button;
		return 0;
}

static void button_read(void)
{
		flex_button_handler temp=flex_button_head;
		uint8_t i=register_button_cnt;
		logic_elec_level=0;
		uint32_t real=0;
		while(temp)
		{
				real|=(temp->flex_usr_read(temp)<<(i-1));
				--i;
				temp=temp->next;
		}
		logic_elec_level=(~real)^real_elec_level;
}

static uint8_t button_process(void){
	uint8_t active_cnt=0;
	flex_button_handler temp=flex_button_head;
	uint8_t i=register_button_cnt;
	while(temp)
	{
		if(temp->status!=FLEX_BTN_STAGE_DEFAULT)
		{
			temp->scan_cnt++;
			active_cnt++;
		}
		switch(temp->status)
		{
			case FLEX_BTN_STAGE_DEFAULT:
			{
				if(ISPRESS(i-1))
				{
					temp->status=FLEX_BTN_STAGE_DOWN;
					SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_DOWN);
				}
				else 
				{
					temp->event=FLEX_BTN_PRESS_NONE;
				}
					temp->scan_cnt=0;
					temp->click_cnt=0;
				break;
			}
			case FLEX_BTN_STAGE_DOWN:
			{
				if(ISPRESS(i-1))
				{
					if(temp->click_cnt>0)
					{
						if(temp->scan_cnt>temp->max_multiple_clicks_interval)
						{
							//按键之前已经进行了click事件
							//，但是现在按下的时间太长了，但是还没有到短按的阈值
							SET_EVENT_ANG_CB(temp,temp->click_cnt
							<FLEX_BTN_PRESS_REPEAT_CLICK?temp->click_cnt
							:FLEX_BTN_PRESS_REPEAT_CLICK);
							temp->status=FLEX_BTN_STAGE_DOWN;
							temp->scan_cnt=0;
							temp->click_cnt=0;
						}
					}
					else if(temp->scan_cnt>=temp->long_hold_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_LONG_HOLD);
					}
					else if(temp->scan_cnt>=temp->long_press_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_LONG_START);
					}
					else if(temp->scan_cnt>=temp->short_press_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_SHORT_START);
					}
					break;
				}
				else 
				{
					if(temp->scan_cnt>=temp->long_hold_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_LONG_HOLD_UP);
						temp->status=FLEX_BTN_STAGE_DEFAULT;
					}
					else if(temp->scan_cnt>temp->long_press_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_LONG_UP);
						temp->status=FLEX_BTN_STAGE_DEFAULT;
					}
					else if(temp->scan_cnt>temp->short_press_tick)
					{
						SET_EVENT_ANG_CB(temp,FLEX_BTN_PRESS_SHORT_UP);
						temp->status=FLEX_BTN_STAGE_DEFAULT;
					}
					else{
						temp->status=FLEX_BTN_STAGE_MULTIPLE_CLICK;
						temp->click_cnt++;
					}
				}
				break;
			}
			case FLEX_BTN_STAGE_MULTIPLE_CLICK:
			{
				if(ISPRESS(i-1))
				{
					temp->status=FLEX_BTN_STAGE_DOWN;
					temp->scan_cnt=0;
				}
				else 
				{
					if(temp->scan_cnt>=temp->max_multiple_clicks_interval)
					{
						SET_EVENT_ANG_CB(temp,temp->click_cnt
						<FLEX_BTN_PRESS_REPEAT_CLICK?temp->click_cnt:
						FLEX_BTN_PRESS_REPEAT_CLICK);
						temp->click_cnt=0;
						temp->status=FLEX_BTN_STAGE_DEFAULT;
						temp->scan_cnt=0;
					}
				}
				break;
			}
			default:
			{
				break;
			}
		}
		temp=temp->next;
		--i;
	}
	return active_cnt;
}

flex_button_event_t flex_button_event_read(flex_button_handler button)
{
	return button->event;
}

static uint8_t flex_button_scan(void)
{
	button_read();
	return button_process();
}

static void flex_button_task(void*p)
{
	uint16_t ms=(uint16_t)p;
	while(1)
	{
		flex_button_scan();
		vTaskDelay(pdMS_TO_TICKS(ms));
	}
}

bool flex_button_init(flex_button_handler button,uint8_t length,flex_button_callback cb,uint16_t ms)
{
	if(button&&cb)
	{
		for(uint8_t i=0;i<length;++i)
		{
			flex_button_register(&button[i],cb);
		}
		gpio_init(button);
		wait_for_reconnect=xSemaphoreCreateBinary();
		xTaskCreate(flex_button_task,"flex_button_task",configMINIMAL_STACK_SIZE,(void*)ms
		,tskIDLE_PRIORITY+4,(void*)&task_flex_button_handle);
		return true;
	}
	return false;
}
