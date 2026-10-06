/**
 ******************************************************************************
 * @file    freertos_hooks.c
 * @brief   FreeRTOS 钩子：栈溢出/内存分配失败处理
 ******************************************************************************
 */

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief  FreeRTOS断言失败：死循环
 * @param  file  文件名
 * @param  line  行号
 * @retval None
 */
void vAssertCalled(const char *file, int line)
{
	(void)file;
	(void)line;
	while (1) {}
}

/**
 * @brief  栈溢出钩子
 * @param  xTask     任务句柄
 * @param  pcTaskName 任务名
 * @retval None
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	(void)pcTaskName;
	configASSERT(0);
}

/**
 * @brief  malloc失败钩子
 * @retval None
 */
void vApplicationMallocFailedHook(void)
{
	configASSERT(0);
}

/**
 * @brief  C库断言失败
 * @param  expr  断言表达式
 * @param  file  文件名
 * @param  line  行号
 * @retval None
 */
void __aeabi_assert(const char *expr, const char *file, int line)
{
	(void)expr;
	(void)file;
	(void)line;
	while (1);
}
