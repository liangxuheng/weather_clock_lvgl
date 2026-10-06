/**
 ******************************************************************************
 * @file    weather.c
 * @brief   天气数据解析：从 ESP32C3 获取 HTTP JSON 响应，提取温度/天气图标
 ******************************************************************************
 */

#include "weather.h"
#include "weather_desc.h"
#include "esp32c3.h"
#include "esp32c3_desc.h"
#include <string.h>
#include <stdio.h>
static bool esp_weather_parse(esp32c3_handler esp32x,weather_handler weather)
{
		memset(weather,0,sizeof(struct weather));
		char*p=strstr(esp32x->recv_buf,"+HTTPCLIENT");
		if(p==NULL)
		{
			return false;
		}
		p=strstr(p,"\"results\":");
		if(!p)
		{
			return false;
		}
		p=strstr(p,"\"location\":");
		if(!p)
		{
			return false;
		}
//		p=strstr(p,"\"id\"");
//		if(!p)
//		{
//			return false;
//		}
		p=strstr(p,"\"name\":");
		if(!p)
		{
			return false;
		}
		uint8_t ret_val=sscanf(p,"\"name\":\"%31[^\"]\""
		,weather->city);
		if(ret_val!=1)
		{
			return false;
		}
		p=strstr(p,"\"path\":");
		if(!p)
		{
			return false;
		}
		ret_val=sscanf(p,"\"path\":\"%127[^\"]\"",weather->loaction);
		if(ret_val!=1)
		{
			return false;
		}
		p=strstr(p,"\"text\":");
		if(p==NULL)
		{
			return false;
		}
		ret_val=sscanf(p,"\"text\":\"%15[^\"]\"",weather->weather);
		if(ret_val!=1)
		{
			return false;
		}
		p=strstr(p,"\"code\":");
		if(!p)
		{
			return false;
		}
		ret_val=sscanf(p,"\"code\":\"%d\"",&weather->weather_code);
		if(ret_val!=1)
		{
			return false;
		}
		p=strstr(p,"\"temperature\":");
		if(p==NULL)
		{
			return false;
		}
		ret_val=sscanf(p,"\"temperature\":\"%f\"",&weather->temperature);
		if(ret_val!=1)
		{
			return false;
		}
		return true;
}

/**
 * @brief  获取天气：HTTP请求API并解析
 * @param  esp32x   ESP32C3句柄
 * @param  weather  天气数据结构体
 * @retval true=成功
 */
bool get_weather(esp32c3_handler esp32x,weather_handler weather)
{
	if(!esp32x||!weather)
	{
		return false;
	}
	return esp_weather_parse(esp32x,weather);
}

const char* get_weather_city(const weather_handler w) {
    return w->city;
}
const char* get_weather_location(const weather_handler w) {
    return w->loaction;
}
const char* get_weather_text(const weather_handler w) {
    return w->weather;
}
/**
 * @brief  获取天气代码
 * @param  w  天气结构体
 * @retval 天气代码
 */
int get_weather_code(const weather_handler w) {
    return w->weather_code;
}
/**
 * @brief  获取温度
 * @param  w  天气结构体
 * @retval 温度
 */
float get_weather_temperature(const weather_handler w) {
    return w->temperature;
}
