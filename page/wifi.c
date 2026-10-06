/**
 ******************************************************************************
 * @file    wifi.c
 * @brief   WiFi 配置：通过 ESP32C3 连接路由器，获取天气 API 数据
 ******************************************************************************
 */

#include "esp32c3.h"
//#include "lcd24.h"
//#include "image.h"
//#include "front.h"
//#include "error_page.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#define DEBUG_FLAG_M 0
 extern volatile QueueHandle_t queue_recv;
 extern volatile SemaphoreHandle_t semaphore_recv;
 extern volatile SemaphoreHandle_t semaphore_send;
bool wifi_init(esp32c3_handler esp32x)
{
	#if DEBUG_FLAG_M
	printf("wifi_init\r\n");
	printf("Free heap before queue create: %u bytes\n", (unsigned int)xPortGetFreeHeapSize());
	#endif
	queue_recv=xQueueCreate(512,sizeof(uint8_t));
	if(queue_recv==NULL)
	{
		printf("queue_recv failed\r\n");
	}
	//configASSERT(queue_recv);
	semaphore_send=xSemaphoreCreateBinary();
	if(semaphore_send==NULL)
	{
		printf("semphore_send failed\r\n");
	}
	//configASSERT(semaphore_send);
	semaphore_recv=xSemaphoreCreateBinary();
	if(semaphore_recv==NULL)
	{
		printf("semaphore_recv failed\r\n");
	}
	//configASSERT(semaphore_recv);
	xTaskCreate(task_recv_parse,"task_recv_parse",configMINIMAL_STACK_SIZE*2
	,(void*)esp32x,tskIDLE_PRIORITY+2,NULL);
	vTaskDelay(pdMS_TO_TICKS(2000));
//	esp_usart_recv_callback_register(esp32x,callback_func);
	if(!esp32_init(esp32x))
	{
		printf("[AT] init failed\r\n");
		return false;
	}
	#if DEBUG_FLAG_M
	else 
	{
		printf("[AT] init success\r\n");
	}
	#endif
	if(!esp_wifi_init(esp32x))
	{
		printf("[WIFI] wifi init failed\r\n");
		return false;
	}
	#if DEBUG_FLAG_M
	else 
	{
		printf("[WIFI] wifi init success\r\n");
	}
	#endif
	if(!esp_at_sntp_init(esp32x))
	{
		printf("[SNTP] sntp init failed\r\n");
		return false;
	}
	#if DEBUG_FLAG_M
	else 
	{
		printf("[SNTP] sntp init success\r\n");
	}
	#endif
	return true;
}


bool wifi_wait_for_connect(esp32c3_handler esp32x,
	const char *ssid,const char*pwd,const char*mac,uint64_t timerout)
{
	if(!esp_wifi_connect(esp32x,ssid,pwd,mac,timerout))
	{
		printf("[WIFI_CONNECT] wifi connect failed\r\n");
		return false;
	}
	#if DEBUG_FLAG_M
	else 
	{
		printf("[WIFI_CONNECT] wifi connect success\r\n");
	}
	#endif
	uint8_t flag=0;
	for(uint8_t i=0;i<10;++i)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
		if(!esp_at_get_wifi_info(esp32x)||!esp_get_wifi_connected(esp32x))
		{
			continue;
		}
		else 
		{
			#if DEBUG_FLAG_M
			printf("[WIFI_IS_CONNECT] wifi connext success\r\n");
			printf("[WIFI_CONNECT] ssid:%s\r\n bssid:%s\r\n channal:%d\r\n rssi:%d\r\n",
			esp_get_wifi_ssid(esp32x),esp_get_wifi_bssid(esp32x),
			esp_get_wifi_channel(esp32x),esp_get_wifi_rssi(esp32x));
			#endif
			flag=1;
			break;
		}
	}
	if(flag)
	{
		return true;
	}
	else 
	{
		printf("[WIFI_IS_CONNECT] wifi connext failed\r\n");
		return false;
	}
}

