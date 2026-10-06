/**
 ******************************************************************************
 * @file    main_loop.c
 * @brief   主循环：定时刷新天气数据、DHT11 温湿度、RTC 时间到 UI
 ******************************************************************************
 */

#include "rtc.h"
#include "rtc_desc.h"
#include "esp32c3.h"
#include "esp32c3_desc.h"
#include "main_page.h"
#include "timer.h"
#include "dht11.h"
#include "weather.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "work_queue.h"
#include <string.h>
#include <stdio.h>
/** 心知天气实时天气接口（key 已脱敏，上传 Git 前请替换为你自己的 API key 与位置ID） */
#define WEATHER_URL "https://api.seniverse.com/v3/weather/now.json?key=YOUR_API_KEY&location=YOUR_LOCATION_ID&language=zh-Hans&unit=c"
#define MILLISECONDS(x) (x)           /**< 毫秒转换宏 */
#define SECONDS(x)      MILLISECONDS((x) * 1000)  /**< 秒转毫秒 */
#define MINUTES(x)      SECONDS((x) * 60)         /**< 分钟转毫秒 */
#define HOURS(x)        MINUTES((x) * 60)          /**< 小时转毫秒 */
#define DAYS(x)          HOURS((x) * 24)          /**< 天转毫秒 */

#define TIME_SYNC_INTERVAL          HOURS(1)   /**< NTP时间同步间隔：1小时 */
#define WIFI_UPDATE_INTERVAL        SECONDS(5) /**< WiFi天气刷新间隔：5秒 */
#define TIME_UPDATE_INTERVAL        SECONDS(1) /**< 时间显示刷新间隔：1秒 */
#define INNER_UPDATE_INTERVAL       SECONDS(3) /**< 室内温湿度刷新间隔：3秒 */
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(1) /**< 室外天气刷新间隔：1分钟 */


struct lcd24;
typedef struct lcd24* lcd24_handler;


static TimerHandle_t timer_sync_handler=NULL;
static TimerHandle_t timer_wifi_updata_handler=NULL;
static TimerHandle_t timer_time_update_handler=NULL;
static TimerHandle_t timer_inner_update_handler=NULL;
static TimerHandle_t timer_outdoor_update_handler=NULL;

static SemaphoreHandle_t semaphore_time_update;

extern lcd24_handler lcd241;
extern dth_handle dht11_1_handler;
extern esp32c3_handler esp32c3_1;
extern rtc_handler rtc_handler_1;
extern weather_handler weather_1;

static void main_loop_time_sync(void);
static void main_loop_wifi_update(void);
static void main_loop_time_update(void);
static void main_loop_inner_update(void);
static void main_loop_outdoor_update(void);

typedef void(*callback_func_t)(void);

/**
 * @brief  工作队列执行函数：执行定时器注册的回调
 * @param  p  函数指针
 * @retval None
 */
static void timer_excute(void*p)
{
	callback_func_t func=(callback_func_t)p;
	func();
}

/**
 * @brief  软件定时器回调：将回调推入工作队列
 * @param  timer  定时器句柄
 * @retval None
 */
static void os_timer_callback_func(TimerHandle_t timer)
{
		callback_func_t func=(callback_func_t)pvTimerGetTimerID(timer);
		work_queue_push(timer_excute,func);
}


/**
 * @brief  时间更新定时器回调：释放信号量给时间更新任务
 * @param  timer  定时器句柄
 * @retval None
 */
void timer_update(TimerHandle_t timer)
{
	xSemaphoreGive(semaphore_time_update);
}

/**
 * @brief  时间更新任务：等待信号量后更新时间显示
 * @param  p  任务参数
 * @retval None
 */
static void time_update_task(void*p)
{
	while(1)
	{
		xSemaphoreTake(semaphore_time_update,portMAX_DELAY);
		main_loop_time_update();
	}
}

/**
 * @brief  主循环初始化：RTC/DHT11/页面/定时器/工作队列
 * @retval None
 */
void main_loop_init(void)
{
	rtc_init();
	dht11_init(dht11_1_handler);
	main_page_show(lcd241,esp32c3_1);
	semaphore_time_update=xSemaphoreCreateBinary();
	timer_sync_handler=xTimerCreate("timer_sync",pdMS_TO_TICKS(1),pdFALSE,
	(void*)main_loop_time_sync,os_timer_callback_func);
	timer_wifi_updata_handler=xTimerCreate("timer_wifi_update",pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL)
	,pdTRUE,(void*)main_loop_wifi_update,os_timer_callback_func);
	timer_time_update_handler=xTimerCreate("timer_time_update",pdMS_TO_TICKS(TIME_UPDATE_INTERVAL)
	,pdTRUE,NULL,timer_update);
	timer_outdoor_update_handler=xTimerCreate("timer_outdoor_update",pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL)
	,pdTRUE,(void*)main_loop_outdoor_update,os_timer_callback_func);
	timer_inner_update_handler=xTimerCreate("timer_inner_update",pdMS_TO_TICKS(INNER_UPDATE_INTERVAL)
	,pdTRUE,(void*)main_loop_inner_update,os_timer_callback_func);
	work_queue_init();
	xTaskCreate(time_update_task,"time_update_task",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+3,NULL);
	work_queue_push(timer_excute,main_loop_time_sync);
	work_queue_push(timer_excute,main_loop_outdoor_update);
	work_queue_push(timer_excute,main_loop_wifi_update);
	work_queue_push(timer_excute,main_loop_inner_update);
	xTimerStart(timer_wifi_updata_handler,1000);
	xTimerStart(timer_time_update_handler,1000);
	xTimerStart(timer_inner_update_handler,1000);
	xTimerStart(timer_outdoor_update_handler,1000);
}

/**
 * @brief  时间同步：通过WiFi从NTP获取并设置RTC
 * @retval None
 */
static void main_loop_time_sync(void)
{
	uint64_t timer_sync_delay=TIME_SYNC_INTERVAL;
	uint16_t year;
	if(!esp_sntp_get_time(esp32c3_1))
	{
		printf("[SNTP] sntp get time failed\r\n");
		timer_sync_delay=SECONDS(1);
		goto err;
	}
	year=esp_get_time_year(esp32c3_1);
	if(year<2000)
	{
		printf("[SNTP] sntp year failed\r\n");
		timer_sync_delay=SECONDS(1);
		goto err;
	}
	rtc_handler_1->year=year;
	rtc_handler_1->month=esp_get_time_month(esp32c3_1);
	rtc_handler_1->day=esp_get_time_day(esp32c3_1);
	rtc_handler_1->hour=esp_get_time_hour(esp32c3_1);
	rtc_handler_1->minute=esp_get_time_minute(esp32c3_1);
	rtc_handler_1->second=esp_get_time_second(esp32c3_1);
	rtc_handler_1->weekday=esp_get_time_weekday(esp32c3_1);
	rtc_set_time(rtc_handler_1);
	err:
	xTimerChangePeriod(timer_sync_handler,timer_sync_delay,0);
}

/**
 * @brief  WiFi天气更新：从API获取室外天气数据
 * @retval None
 */
static void main_loop_wifi_update(void)
{
	static struct esp_wifi_info esp_last_wifi_info={0};
	xTimerChangePeriod(timer_wifi_updata_handler,WIFI_UPDATE_INTERVAL,0);
	if(!esp_at_get_wifi_info(esp32c3_1)||\
		!esp_get_wifi_connected(esp32c3_1))
	{
		printf("[WIFI] wifi connect failed\r\n");
		esp_last_wifi_info.ssid="wifi lost";
		main_page_redraw_wifi_ssid(lcd241,esp_last_wifi_info.ssid);
		return;
	}
	const char* ssid=esp_get_wifi_ssid(esp32c3_1);
	if(!ssid)
	{
		printf("[WIFI] wifi get ssid failed\r\n");
		esp_last_wifi_info.ssid="wifi lost";
		main_page_redraw_wifi_ssid(lcd241,esp_last_wifi_info.ssid);
		return;
	}
	if(strcmp(ssid,esp_last_wifi_info.ssid)==0)
	{
		return;
	}
	memcpy(&esp_last_wifi_info,
	esp32c3_1->esp_wifi_info_pointer,
	sizeof(struct esp_wifi_info));
	main_page_redraw_wifi_ssid(lcd241,ssid);
}

/**
 * @brief  时间更新：读取RTC并刷新页面时间显示
 * @retval None
 */
static void main_loop_time_update(void)
{
	static struct rtc_struct rtc_last_info={0};
	xTimerChangePeriod(timer_time_update_handler,TIME_UPDATE_INTERVAL,0);
	rtc_get_time(rtc_handler_1);
	if(rtc_handler_1->year<2000)
	{
		printf("[RTC] rtc get time failed\r\n");
		return;
	}
	if(memcmp(&rtc_last_info,rtc_handler_1,\
		sizeof(struct rtc_struct))==0)
		{
			return;
		}
		memcpy(&rtc_last_info,rtc_handler_1,sizeof(struct rtc_struct));
		main_page_redraw_date(lcd241,rtc_handler_1);
		main_page_redraw_time(lcd241,rtc_handler_1);
}

/**
 * @brief  室内数据更新：读取DHT11温湿度并刷新页面
 * @retval None
 */
static void main_loop_inner_update(void)
{
	static uint8_t recv_last_buf[5]={0};
	xTimerChangePeriod(timer_inner_update_handler,INNER_UPDATE_INTERVAL,0);
	uint8_t recv_buf[5]={0};
	if(!dht11_data_read(dht11_1_handler,recv_buf,sizeof(recv_buf)))
	{
		printf("[INNER] inner get failed\r\n");
		return;
	}
	if(recv_buf[2]!=recv_last_buf[2])
	{
		main_page_redraw_inner_temperature(lcd241,(float)recv_buf[2]);
		recv_last_buf[2]=recv_buf[2];
	}
	if(recv_buf[0]!=recv_last_buf[0])
	{
		main_page_redraw_inner_humidity(lcd241,(float)recv_buf[0]);
		recv_last_buf[0]=recv_buf[0];
	}

}

/**
 * @brief  室外数据更新：刷新页面室外天气图标
 * @retval None
 */
static void main_loop_outdoor_update(void)
{
	static char*last_city=NULL;
	static int last_code=-1;
	static float last_temperature=-1.0f;
	xTimerChangePeriod(timer_outdoor_update_handler,OUTDOOR_UPDATE_INTERVAL,0);
	if(!esp_http_get(esp32c3_1,WEATHER_URL))
	{
		printf("[outdoor] outdoor get weather failed\r\n");
		return;
	}
	if(!get_weather(esp32c3_1,weather_1))
	{
		printf("[outdoor] outdoor parse weather failed\r\n");
		return;
	}
	char*city=NULL;
	city=(char*)get_weather_city(weather_1);
	int code=get_weather_code(weather_1);
	float temperature=-1.0f;
	temperature=get_weather_temperature(weather_1);
	if(city==NULL||temperature==-1.0f)
	{
		printf("[ourdoor] outdoor weather error\r\n");
		return;
	}
	if(strcmp(city,last_city))
	{
		main_page_redraw_outdoor_city(lcd241,city);
		strcpy(last_city,city);
	}
	if(code!=last_code)
	{
		main_page_redraw_outdoor_weather_icon(lcd241,code);
		last_code=code;
	}
	if(temperature!=last_temperature)
	{
		main_page_redraw_outdoor_temperature(lcd241,temperature);
		last_temperature=temperature;
	}
}


