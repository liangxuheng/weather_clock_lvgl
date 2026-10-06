/**
 ******************************************************************************
 * @file    main.c
 * @brief   天气时钟入口：初始化 UI/WiFi/传感器，启动 FreeRTOS
 ******************************************************************************
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "board.h"
#include "delay.h"
#include "cmd_queue.h"
#include "main_loop.h"
#include "ui.h"
#include "welcome_page.h"
#include "wifi.h"
#include "wifi_page.h"
#include "error_page.h"
#include "flex_key.h"
#include "app_ctx.h"

#define WIFI_NAME "your_wifi_ssid"       /**< WiFi SSID */
#define WIFI_PASSWORD "your_wifi_password" /**< WiFi密码 */
#define DEBUG 0                           /**< 调试打印开关：1=开, 0=关 */

extern SemaphoreHandle_t wait_for_reconnect;
extern TaskHandle_t task_flex_button_handle;

/* 应用上下文：board_init() 后填充一次，业务模块通过它拿句柄 */
static weather_app_t g_app;

/**
 * @brief  按键回调：按键释放时触发WiFi重连
 * @param  key_x  按键句柄
 * @retval None
 */
static void key_callback_func(void *key_x)
{
    flex_button_handler temp = (flex_button_handler)key_x;
    flex_button_event_t event = flex_button_event_read(temp);
    if (event == FLEX_BTN_PRESS_SHORT_UP ||
        event == FLEX_BTN_PRESS_LONG_UP ||
        event == FLEX_BTN_PRESS_LONG_HOLD_UP ||
        event == FLEX_BTN_PRESS_CLICK)
    {
        xSemaphoreGive(wait_for_reconnect);
    }
}

/**
 * @brief  UI初始化：LCD+欢迎页
 * @retval None
 */
static void ui_app_init(void)
{
    ui_init(lcd241, USART_desc_1);
    welcome_page_init();
    welcome_page_show(lcd241, image_heiqiyihu_welcome_handle, front_32X32, front_24X24);
}

/**
 * @brief  WiFi连接：初始化ESP32C3并连接指定热点，失败等待按键重连
 * @retval true=连接成功
 */
static bool wifi_app_connect(void)
{
    bool ret = wifi_init(esp32c3_1);
    wifi_page_show(lcd241, front_20X20, front_32X32, image_sihuang_wifi_handle);

    if (!ret)
    {
        error_page_show(lcd241, "error timeout", image_zuozhu_error_handle, front_20X20);
        flex_button_init(flex_handler_1, 1, key_callback_func, 20);
        while (1)
        {
            xSemaphoreTake(wait_for_reconnect, portMAX_DELAY);
            ret = wifi_init(esp32c3_1);
            if (ret)
            {
                wifi_page_show(lcd241, front_20X20, front_32X32, image_sihuang_wifi_handle);
                break;
            }
        }
    }

    ret = wifi_wait_for_connect(esp32c3_1, WIFI_NAME, WIFI_PASSWORD, NULL, 5000);
    if (!ret)
    {
        error_page_show(lcd241, "error timeout", image_zuozhu_error_handle, front_20X20);
        flex_button_init(flex_handler_1, 1, key_callback_func, 20);
        while (1)
        {
            xSemaphoreTake(wait_for_reconnect, portMAX_DELAY);
            ret = wifi_wait_for_connect(esp32c3_1, WIFI_NAME, WIFI_PASSWORD, NULL, 5000);
            if (ret)
            {
                wifi_page_show(lcd241, front_20X20, front_32X32, image_sihuang_wifi_handle);
                vTaskDelete(task_flex_button_handle);
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief  初始化任务：UI→WiFi→主循环
 * @param  args  任务参数(未使用)
 * @retval None
 */
static void init_task(void *args)
{
    (void)args;
    ui_app_init();

#if DEBUG
    uint32_t tick_before = xTaskGetTickCount();
    printf("\r\n[DEBUG] Tick Before Delay: %lu\r\n", tick_before);
    vTaskDelay(pdMS_TO_TICKS(2000));
    uint32_t tick_after = xTaskGetTickCount();
    printf("[DEBUG] Tick After Delay: %lu\r\n", tick_after);
#endif

    wifi_app_connect();
    main_loop_init(&g_app);
    vTaskDelete(NULL);
}

int main(void)
{
    board_init();
    /* 一次性打包句柄到 ctx（board.h 只做前向声明，此处句柄来自各模块全局定义） */
    g_app.lcd     = lcd241;
    g_app.esp     = esp32c3_1;
    g_app.rtc     = rtc_handler_1;
    g_app.weather = weather_1;
    g_app.dht     = dht11_1_handler;
    g_app.key     = flex_handler_1;
    xTaskCreate(init_task, "init_task", configMINIMAL_STACK_SIZE * 2,
        NULL, tskIDLE_PRIORITY + 3, NULL);
    vTaskStartScheduler();
    while (1)
    {
    }
}
