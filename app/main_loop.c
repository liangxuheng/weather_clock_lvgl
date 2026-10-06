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
#include "app_ctx.h"
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


/* main.c 经 main_loop_init 传入的 ctx；本文件内部刷新回调均为无参定时器回调，
   故用模块私有指针持有，不再直接 extern 全局句柄 */
static weather_app_t *g_loop_app = NULL;

static TimerHandle_t timer_sync_handler=NULL;
static TimerHandle_t timer_wifi_updata_handler=NULL;
static TimerHandle_t timer_time_update_handler=NULL;
static TimerHandle_t timer_inner_update_handler=NULL;
static TimerHandle_t timer_outdoor_update_handler=NULL;

static SemaphoreHandle_t semaphore_time_update;

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
 * @param  app  应用上下文（main.c 填充好的句柄集合）
 * @retval None
 */
void main_loop_init(weather_app_t *app)
{
	g_loop_app = app;
	rtc_init();
	dht11_init(g_loop_app->dht);
	main_page_show(g_loop_app->lcd,g_loop_app->esp);
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
	if(!esp_sntp_get_time(g_loop_app->esp))
	{
		printf("[SNTP] sntp get time failed\r\n");
		timer_sync_delay=SECONDS(1);
		goto err;
	}
	year=esp_get_time_year(g_loop_app->esp);
	if(year<2000)
	{
		printf("[SNTP] sntp year failed\r\n");
		timer_sync_delay=SECONDS(1);
		goto err;
	}
	g_loop_app->rtc->year=year;
	g_loop_app->rtc->month=esp_get_time_month(g_loop_app->esp);
	g_loop_app->rtc->day=esp_get_time_day(g_loop_app->esp);
	g_loop_app->rtc->hour=esp_get_time_hour(g_loop_app->esp);
	g_loop_app->rtc->minute=esp_get_time_minute(g_loop_app->esp);
	g_loop_app->rtc->second=esp_get_time_second(g_loop_app->esp);
	g_loop_app->rtc->weekday=esp_get_time_weekday(g_loop_app->esp);
	rtc_set_time(g_loop_app->rtc);
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
	if(!esp_at_get_wifi_info(g_loop_app->esp)||\
		!esp_get_wifi_connected(g_loop_app->esp))
	{
		printf("[WIFI] wifi connect failed\r\n");
		esp_last_wifi_info.ssid="wifi lost";
		main_page_redraw_wifi_ssid(g_loop_app->lcd,esp_last_wifi_info.ssid);
		return;
	}
	const char* ssid=esp_get_wifi_ssid(g_loop_app->esp);
	if(!ssid)
	{
		printf("[WIFI] wifi get ssid failed\r\n");
		esp_last_wifi_info.ssid="wifi lost";
		main_page_redraw_wifi_ssid(g_loop_app->lcd,esp_last_wifi_info.ssid);
		return;
	}
	if(strcmp(ssid,esp_last_wifi_info.ssid)==0)
	{
		return;
	}
	memcpy(&esp_last_wifi_info,
	g_loop_app->esp->esp_wifi_info_pointer,
	sizeof(struct esp_wifi_info));
	main_page_redraw_wifi_ssid(g_loop_app->lcd,ssid);
}

/**
 * @brief  时间更新：读取RTC并刷新页面时间显示
 * @retval None
 */
static void main_loop_time_update(void)
{
	static struct rtc_struct rtc_last_info={0};
	xTimerChangePeriod(timer_time_update_handler,TIME_UPDATE_INTERVAL,0);
	rtc_get_time(g_loop_app->rtc);
	if(g_loop_app->rtc->year<2000)
	{
		printf("[RTC] rtc get time failed\r\n");
		return;
	}
	if(memcmp(&rtc_last_info,g_loop_app->rtc,\
		sizeof(struct rtc_struct))==0)
		{
			return;
		}
		memcpy(&rtc_last_info,g_loop_app->rtc,sizeof(struct rtc_struct));
		main_page_redraw_date(g_loop_app->lcd,g_loop_app->rtc);
		main_page_redraw_time(g_loop_app->lcd,g_loop_app->rtc);
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
	if(!dht11_data_read(g_loop_app->dht,recv_buf,sizeof(recv_buf)))
	{
		printf("[INNER] inner get failed\r\n");
		return;
	}
	if(recv_buf[2]!=recv_last_buf[2])
	{
		main_page_redraw_inner_temperature(g_loop_app->lcd,(float)recv_buf[2]);
		recv_last_buf[2]=recv_buf[2];
	}
	if(recv_buf[0]!=recv_last_buf[0])
	{
		main_page_redraw_inner_humidity(g_loop_app->lcd,(float)recv_buf[0]);
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
	if(!esp_http_get(g_loop_app->esp,WEATHER_URL))
	{
		printf("[outdoor] outdoor get weather failed\r\n");
		return;
	}
	if(!get_weather(g_loop_app->esp,g_loop_app->weather))
	{
		printf("[outdoor] outdoor parse weather failed\r\n");
		return;
	}
	char*city=NULL;
	city=(char*)get_weather_city(g_loop_app->weather);
	int code=get_weather_code(g_loop_app->weather);
	float temperature=-1.0f;
	temperature=get_weather_temperature(g_loop_app->weather);
	if(city==NULL||temperature==-1.0f)
	{
		printf("[ourdoor] outdoor weather error\r\n");
		return;
	}
	if(strcmp(city,last_city))
	{
		main_page_redraw_outdoor_city(g_loop_app->lcd,city);
		strcpy(last_city,city);
	}
	if(code!=last_code)
	{
		main_page_redraw_outdoor_weather_icon(g_loop_app->lcd,code);
		last_code=code;
	}
	if(temperature!=last_temperature)
	{
		main_page_redraw_outdoor_temperature(g_loop_app->lcd,temperature);
		last_temperature=temperature;
	}
}


