/**
 ******************************************************************************
 * @file    ui.c
 * @brief   LVGL UI 管理：页面切换、消息队列分发
 ******************************************************************************
 */

#include "ui.h"
#include "ui_desc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "USART.h"
#include <stdio.h>
#include <string.h>
#include "lvgl/lvgl.h"
#define DE_BUGE_FLAG 0
//struct USART_desc;
//typedef  struct USART_desc* USART_MY_handle;
//void lcd_init(lcd24_handler lcd24_x);
//void lcd_fill_colour(lcd24_handler lcd24_x,uint16_t x1\
//	,uint16_t x2,uint16_t y1,uint16_t y2,uint16_t colour );
//void lcd_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
//	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs,uint16_t colour_bg);
//void lcd_show_image(lcd24_handler lcd24x,uint16_t x,uint16_t y,image_handle imagex);
//struct ui_message
//{
//	ui_message_t action;
//	union{
//		struct ui_fill_colour_structer{
//			lcd24_handler lcd24x;
//			uint16_t x1;
//			uint16_t x2;
//			uint16_t y1;
//			uint16_t y2;
//			uint16_t colour;
//		};
//		struct ui_show_str_structer{
//			lcd24_handler lcd24y;
//			uint16_t x;
//			uint16_t y;
//			front_handler frontx;
//			char*str;
//			uint16_t legth;
//			uint16_t colour_asc;
//			uint16_t colour_bg;
//		};
//		struct ui_show_image_structer{
//		lcd24_handler lcd24z;
//			uint16_t _x;
//			uint16_t _y;
//			image_handle imagex;
//		};
//	};
//};



static QueueHandle_t queue_ui=NULL;
//static struct ui_message message_struct={0}; 
//static ui_message_handler message_ui_handler=&message_struct;

/**
 * @brief  发送矩形填充消息到UI队列
 * @param  lcd24x  LCD句柄
 * @param  x1,x2,y1,y2  矩形区域
 * @param  colour  填充色
 * @retval None
 */
void ui_set_fill_colour(lcd24_handler lcd24x,uint16_t x1,uint16_t x2,uint16_t y1\
	,uint16_t y2,uint16_t colour)
{
		struct ui_message message_struct={0};
		message_struct.action=UI_FILL_COLOUR;
		message_struct.ui_fill_colour_structer.lcd24x=lcd24x;
		message_struct.ui_fill_colour_structer.colour=colour;
		message_struct.ui_fill_colour_structer.x1=x1;
		message_struct.ui_fill_colour_structer.x2=x2;
		message_struct.ui_fill_colour_structer.y1=y1;
		message_struct.ui_fill_colour_structer.y2=y2;
		xQueueSend(queue_ui,&message_struct,0);
}

/**
 * @brief  发送字符串显示消息到UI队列
 * @param  lcd24x    LCD句柄
 * @param  x,y       坐标
 * @param  frontx    字体
 * @param  str       字符串
 * @param  length    长度
 * @param  colour_acs 字体色
 * @param  colour_bg   背景色
 * @retval None
 */
void ui_set_show_str(lcd24_handler lcd24x,uint16_t x,uint16_t y\
	,front_handler frontx,char*str,uint32_t length,uint16_t colour_acs\
		,uint16_t colour_bg)
{
	struct ui_message message_struct={0};
	//因为这里有指针，所以要避免指针悬空或者失效的问题
	message_struct.ui_show_str_structer.str=pvPortMalloc(strlen(str)+1);
	if(message_struct.ui_show_str_structer.str==NULL)
	{
		printf("malloc failed\r\n");
		return;
	}
	strcpy(message_struct.ui_show_str_structer.str,str);
	message_struct.action=UI_SHOW_STR;
	message_struct.ui_show_str_structer.colour_asc=colour_acs;
	message_struct.ui_show_str_structer.colour_bg=colour_bg;
	message_struct.ui_show_str_structer.frontx=frontx;
	message_struct.ui_show_str_structer.lcd24y=lcd24x;
	message_struct.ui_show_str_structer.legth=strlen(str);
	message_struct.ui_show_str_structer.x=x;
	message_struct.ui_show_str_structer.y=y;
	xQueueSend(queue_ui,&message_struct,0);
}

/**
 * @brief  发送图片显示消息到UI队列
 * @param  lcd24x  LCD句柄
 * @param  x,y     坐标
 * @param  imagex  图片句柄
 * @retval None
 */
void ui_set_show_image(lcd24_handler lcd24x,uint16_t x,uint16_t y,image_handle imagex)
{
	struct ui_message message_struct={0};
	message_struct.action=UI_SHOW_IMAGE;
	message_struct.ui_show_image_structer._x=x;
	message_struct.ui_show_image_structer._y=y;
	message_struct.ui_show_image_structer.lcd24z=lcd24x;
	message_struct.ui_show_image_structer.imagex=imagex;
	xQueueSend(queue_ui,&message_struct,0);
}

static void ui_task(void*p)
{
	while(1)
	{
		#if DE_BUGE_FLAG
		printf("enter ui\r\n");
		#endif
		struct ui_message temp={0};
		while(xQueueReceive(queue_ui,&temp,0)==pdTRUE){
		//xQueueReceive(queue_ui,&temp,portMAX_DELAY);
		switch(temp.action)
		{
			case UI_FILL_COLOUR:
			{
				lcd_fill_colour(temp.ui_fill_colour_structer.lcd24x
				,temp.ui_fill_colour_structer.x1,temp.ui_fill_colour_structer.x2
				,temp.ui_fill_colour_structer.y1,temp.ui_fill_colour_structer.y2,
				temp.ui_fill_colour_structer.colour);
				break;
			}
			case UI_SHOW_STR:
			{
				lcd_show_str(temp.ui_show_str_structer.lcd24y
				,temp.ui_show_str_structer.x,temp.ui_show_str_structer.y,
				temp.ui_show_str_structer.frontx,temp.ui_show_str_structer.str,
				temp.ui_show_str_structer.legth,temp.ui_show_str_structer.colour_asc,
				temp.ui_show_str_structer.colour_bg);
				vPortFree(temp.ui_show_str_structer.str);
				break;
			}
			case UI_SHOW_IMAGE:
			{
				lcd_show_image(temp.ui_show_image_structer.lcd24z
				,temp.ui_show_image_structer._x,temp.ui_show_image_structer._y,
				temp.ui_show_image_structer.imagex);
				break;
			}
			default:{
				printf("no match\r\n");
			}
		}
	}
		lv_timer_handler();
	vTaskDelay(pdMS_TO_TICKS(5));
	}
}

/**
 * @brief  UI初始化：串口+LCD+消息队列+UI任务
 * @param  lcd24x  LCD句柄
 * @param  usartx  串口句柄
 * @retval None
 */
void ui_init(lcd24_handler lcd24x,USART_MY_handle usartx)
{
		usart_init(usartx);
		lcd_init(lcd24x);
		queue_ui=xQueueCreate(16,sizeof(struct ui_message));
		configASSERT(queue_ui);
		#if DE_BUGE_FLAG
		printf("Free heap before create UI task: %u\n", xPortGetFreeHeapSize());
		#endif
		xTaskCreate(ui_task,"ui_task",configMINIMAL_STACK_SIZE*8,NULL
		,tskIDLE_PRIORITY+4,NULL);
}

