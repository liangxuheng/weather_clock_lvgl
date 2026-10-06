/**
 ******************************************************************************
 * @file    work_queue.c
 * @brief   工作队列：慢操作投递到低优先级任务执行
 ******************************************************************************
 */

#include "work_queue.h"
#include "work_queue_desc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static QueueHandle_t queue_handler=NULL;

static void work_queue_task(void *p)
{
	while(1)
	{
			struct work_queue_item temp={0};
			xQueueReceive(queue_handler,&temp,portMAX_DELAY);
			temp.work(temp.args);
	}
}

/**
 * @brief  工作队列初始化：创建队列和后台任务
 * @retval None
 */
void work_queue_init(void)
{
	queue_handler=xQueueCreate(32,sizeof(struct work_queue_item));
	configASSERT(queue_handler);
	xTaskCreate(work_queue_task,"work_queue_task"
	,configMINIMAL_STACK_SIZE*2,NULL,tskIDLE_PRIORITY+2,NULL);
}

/**
 * @brief  将慢操作推入工作队列执行
 * @param  func_callback  回调函数
 * @param  args           参数
 * @retval None
 */
void work_queue_push(work_queue_callback_t func_callback,void*args)
{
	struct work_queue_item temp={0};
	if(func_callback&&args)
	{
		temp.work=func_callback;
		temp.args=args;
		xQueueSend(queue_handler,&temp,0);
	}
}
