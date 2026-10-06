/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   STM32 RTC 驱动：时间读写，BKP 寄存器掉电保持
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include "rtc.h"
#include "rtc_desc.h"
#include <string.h>
#include <stdio.h>
/**
 * @brief  RTC初始化：使能LSE并同步
 * @retval None
 */
void rtc_init(void){
    RTC_InitTypeDef RTC_InitStruct;
    RTC_StructInit(&RTC_InitStruct);
    RTC_Init(&RTC_InitStruct);
    
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForSynchro();
}

static void rtc_get_time_once(rtc_handler rtcx){
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    
    RTC_DateStructInit(&date);
    RTC_TimeStructInit(&time);
		
		RTC_GetDate(RTC_Format_BIN, &date);
    RTC_GetTime(RTC_Format_BIN, &time);
    
    rtcx->year = 2000 + date.RTC_Year;
    rtcx->month = date.RTC_Month;
    rtcx->day = date.RTC_Date;
    rtcx->weekday = date.RTC_WeekDay;
    rtcx->hour = time.RTC_Hours;
    rtcx->minute = time.RTC_Minutes;
    rtcx->second = time.RTC_Seconds;
		
}

/**
 * @brief  读取RTC时间（两次读校验一致性）
 * @param  rtcx  RTC句柄
 * @retval None
 */
void rtc_get_time(rtc_handler rtcx){
	struct rtc_struct rtc_temp;
	do{
		rtc_get_time_once(&rtc_temp);
		rtc_get_time_once(rtcx);
	}while(memcmp(&rtc_temp,rtcx,sizeof(struct rtc_struct)));
	//printf("weekday:%d\r\n",rtcx->weekday);
}

static void rtc_set_time_once(rtc_handler rtcx){
	  RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    
    RTC_DateStructInit(&date);
    RTC_TimeStructInit(&time);
	
	  date.RTC_Year = rtcx->year - 2000;
    date.RTC_Month = rtcx->month;
    date.RTC_Date = rtcx->day;
    date.RTC_WeekDay = rtcx->weekday;
    time.RTC_Hours = rtcx->hour;
    time.RTC_Minutes = rtcx->minute;
    time.RTC_Seconds = rtcx->second;
    
    RTC_SetDate(RTC_Format_BIN, &date);
    RTC_SetTime(RTC_Format_BIN, &time);
    
}

/**
 * @brief  设置RTC时间（写后回读校验）
 * @param  rtcx  RTC句柄
 * @retval None
 */
void rtc_set_time(rtc_handler rtcx){
	struct rtc_struct rtc_temp;
	do{
		rtc_set_time_once(rtcx);
		rtc_get_time_once(&rtc_temp);
	}while(rtcx->second!=rtc_temp.second);
	//printf("weekday:%d\r\n",rtcx->weekday);
}
