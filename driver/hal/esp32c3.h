#ifndef __ESP32C3_H_
#define __ESP32C3_H_
#include <stdbool.h>
#include <stdint.h>
struct m_queue;
typedef  struct m_queue* queue_handle;
typedef void  (*usart_callback_t)(queue_handle queuex,char c);
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
bool esp32_init(esp32c3_handler esp32c3x);
bool esp_at_write_command(esp32c3_handler esp32c3x,
	const char*cmd,uint64_t timeout);
void esp_usart_recv_callback_register(esp32c3_handler esp32c3x
	,usart_callback_t func);
bool esp_wifi_init(esp32c3_handler esp32x);
bool esp_wifi_connect(esp32c3_handler esp32x\
	,const char*ssid,const char*pwd,const char*mac,uint64_t timeout);
bool esp_at_get_wifi_info(esp32c3_handler esp32x);
bool esp_sntp_get_time(esp32c3_handler esp32x);
bool esp_http_get(esp32c3_handler esp32x,const char*url);
bool esp_at_sntp_init(esp32c3_handler esp32x);
// WiFi 信息
const char* esp_get_wifi_ssid(const esp32c3_handler esp32x);
const char* esp_get_wifi_bssid(const esp32c3_handler esp32x);
int32_t esp_get_wifi_rssi(const esp32c3_handler esp32x);
int32_t esp_get_wifi_channel(const esp32c3_handler esp32x);
bool esp_get_wifi_connected(const esp32c3_handler esp32x);

// SNTP 时间
uint16_t esp_get_time_year(const esp32c3_handler esp32x);
uint8_t esp_get_time_month(const esp32c3_handler esp32x);
uint8_t esp_get_time_day(const esp32c3_handler esp32x);
uint8_t esp_get_time_hour(const esp32c3_handler esp32x);
uint8_t esp_get_time_minute(const esp32c3_handler esp32x);
uint8_t esp_get_time_second(const esp32c3_handler esp32x);
uint8_t esp_get_time_weekday(const esp32c3_handler esp32x);


//接收和解析的任务
void task_recv_parse(void*p);
#endif
