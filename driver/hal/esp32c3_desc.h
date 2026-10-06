#ifndef __EPS32C3_DESC_H_
#define __EPS32C3_DESC_H_
#include <stdint.h>
#include <stdbool.h>
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
struct m_queue;
typedef  struct m_queue* queue_handle;
struct esp_wifi_info{
	char * ssid;
	uint8_t ssid_len;
	char *bssid;
	uint8_t bssid_len;
	int32_t channal;
	int32_t rssi;
	char *pwd;//密码
	uint8_t pwd_len;
	bool connected;
};
typedef struct esp_wifi_info* esp_wifi_info_handler;
struct esp_data_struct{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
};
typedef struct esp_data_struct* esp_data_struct_handler;
struct esp32_c3
{
	USART_MY_handle usartx;
	char* send_cmd;
	uint16_t send_len;
	char *recv_buf;
	//uint8_t index;
//	char*recv_index;
	uint32_t recv_len;
	//queue_handle recv_queue;
	esp_wifi_info_handler esp_wifi_info_pointer;
	esp_data_struct_handler esp_data_struct_pointer;
};
#endif
