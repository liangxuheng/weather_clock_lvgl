#ifndef __APP_CTX_H_
#define __APP_CTX_H_

struct lcd24;
typedef struct lcd24* lcd24_handler;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
struct rtc_struct;
typedef struct rtc_struct* rtc_handler;
struct weather;
typedef struct weather* weather_handler;
struct dht11_struct;
typedef struct dht11_struct* dth_handle;
struct flex_button;
typedef struct flex_button* flex_button_handler;

/* 应用上下文：main_loop 需要的句柄打包，main.c 填充一次后经 main_loop_init 传入 */
typedef struct {
    lcd24_handler       lcd;      /* LCD 句柄（原全局 lcd241） */
    esp32c3_handler     esp;      /* ESP32C3 WiFi 模块（原 esp32c3_1） */
    rtc_handler         rtc;      /* RTC（原 rtc_handler_1） */
    weather_handler     weather;  /* 天气数据缓存（原 weather_1） */
    dth_handle          dht;      /* DHT11 温湿度（原 dht11_1_handler） */
    flex_button_handler key;      /* 按键（原 flex_handler_1） */
} weather_app_t;

#endif
