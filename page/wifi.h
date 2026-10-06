#ifndef __WIFI_H_
#define __WIFI_H_
#include <stdbool.h>
#include <stdint.h>
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
struct m_queue;
typedef  struct m_queue* queue_handle;
typedef void  (*usart_callback_t)(queue_handle queuex,char c);
bool wifi_init(esp32c3_handler esp32x);
bool wifi_wait_for_connect(esp32c3_handler esp32x,
	const char *ssid,
		const char*pwd,const char*mac,uint64_t timerout);
#endif
