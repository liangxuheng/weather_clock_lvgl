/**
 ******************************************************************************
 * @file    esp32c3.c
 * @brief   ESP32C3 WiFi 模块驱动：AT 指令连接路由、HTTP GET 天气数据
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include "esp32c3_desc.h"
#include "esp32c3.h"
#include "usart.h"
#include "usart_desc.h"
//#include "core_cm4.h"  // 确保可以使用 NVIC_GetPriority
//#include "cmd_queue.h"
//#include "queue_desc.h"
#include <string.h>
#include <stdio.h>
#include "timer.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#define DEBUG_ESP32 0
#define ARR_SIZE (sizeof(at_ack_matches)/sizeof(at_ack_match_t))
 volatile QueueHandle_t queue_recv=NULL;
 volatile SemaphoreHandle_t semaphore_recv=NULL;
 volatile SemaphoreHandle_t semaphore_send=NULL;
typedef enum
{
    AT_ACK_NONE,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY,
} at_ack_t;

typedef struct
{
    at_ack_t ack;
    const char *string;
} at_ack_match_t;

static const at_ack_match_t at_ack_matches[] = 
{
    {AT_ACK_OK, "OK\r\n"},
    {AT_ACK_ERROR, "ERROR\r\n"},
    {AT_ACK_BUSY, "busy p..\r\n"},
    {AT_ACK_READY, "ready\r\n"},
};

static at_ack_t ack_match(esp32c3_handler esp32x);

static at_ack_t ack_return=AT_ACK_NONE;
//接收和解析的任务
/**
 * @brief  ESP32C3 串口数据接收解析任务
 * @param  p  esp32c3句柄
 * @retval None
 */
void task_recv_parse(void*p)
{

	uint8_t ch=0;
	esp32c3_handler esp32x=(esp32c3_handler)p;
	char*p_buf=esp32x->recv_buf;
	uint16_t index=0;
	uint8_t state=0;
//	uint16_t offset=0;
	while(1)
	{
		if(xQueueReceive(queue_recv,&ch,portMAX_DELAY))
		{
			if(esp32x->recv_buf[0]==0)
			{
//			p=esp32x->recv_buf;
//			offset=0;
				index=0;
				state=0;
			}
			//printf("recv\r\n");
			if(index<esp32x->recv_len-1)
			{
				p_buf[index++]=ch;
			}
				if(state==0&&ch=='\r')
			{
				state=1;
			}
			else if(state==1&&ch=='\n')
			{
				
					#if DEBUG_ESP32
					printf("recv_buf:%s\r\n",esp32x->recv_buf);
					#endif
					ack_return=ack_match(esp32x);
					if(ack_return!=AT_ACK_NONE)
					{
						index=0;

						xSemaphoreGive(semaphore_recv);
						while(p_buf[0])
						{
							vTaskDelay(pdMS_TO_TICKS(1));
						}
					}
//					p+=strlen(esp32x->recv_buf)-offset;
//					offset+=strlen(esp32x->recv_buf);
						state=0;

			}
		}
	}
}


static at_ack_t ack_match(esp32c3_handler esp32x)
{
	if(esp32x->recv_buf[0]=='\0')
	{
		return AT_ACK_NONE;
	} 
	for(uint8_t i=0;i<ARR_SIZE;++i)
	{
		if(strstr(esp32x->recv_buf,\
			at_ack_matches[i].string))
		{
			return at_ack_matches[i].ack;
		}
	}
	return AT_ACK_NONE;
}

static void set_send_cmd(esp32c3_handler esp32c3x,const char*cmd\
	,uint16_t len)
{
			memset(esp32c3x->send_cmd,0,esp32c3x->send_len);
	if(cmd&&len>0)
	{
		snprintf(esp32c3x->send_cmd,esp32c3x->send_len,"%s\r\n",cmd);
		#if DEBUG_ESP32
		printf("cmd:%s\r\n",esp32c3x->send_cmd);
		#endif
	}
}

static at_ack_t esp_basic_response(uint64_t timer_out)
{
//	//m_queue_reset(esp32x->recv_queue);
//	//uint64_t last=tick_now_tick();
//	uint64_t last=xTaskGetTickCount();
//	uint32_t index=0;
//	uint8_t state=0;
//	uint16_t offset=0;
//	//memset(esp32x->recv_buf,0,esp32x->recv_len);
//	char *p_temp=esp32x->recv_buf;
//	while(xTaskGetTickCount()-last<timerout/portTICK_PERIOD_MS&&index<esp32x->recv_len-offset)
//	{
//		int8_t ret_val_data=m_queue_top(esp32x->recv_queue);
//		if(ret_val_data!=ERROR_NUM)
//		{
//			m_queue_pop(esp32x->recv_queue);
//			p_temp[index++]=ret_val_data;
//			if(state==0&&ret_val_data=='\r')
//			{
//				state=1;
//			}
//			else if(state==1&&ret_val_data=='\n')
//			{
//				at_ack_t ack=ack_match(esp32x);
//				if(ack!=AT_ACK_NONE)
//				{
//					#if DEBUG_ESP32
//					printf("recv_buf:%s\r\n",p_temp);
//					#endif	
//					return ack;
//				}
//				//memset(esp32x->recv_buf,0,index*sizeof(char));
//				p_temp+=strlen(esp32x->recv_buf)-offset;
//				offset=strlen(esp32x->recv_buf);
//				//esp32x->recv_index=p_temp;
//				index=0;
//				state=0;
//			}
//		}
//		else 
//		{
//			//delay(10);
//			vTaskDelay(pdMS_TO_TICKS(1));
//		}
//	}
		BaseType_t ret=xSemaphoreTake(semaphore_recv,timer_out);
		return ret?ack_return:AT_ACK_NONE;
}
static at_ack_t esp_write_by_usart(esp32c3_handler esp32x,uint64_t timerout)
{
	uint32_t length=strlen(esp32x->send_cmd);
	char*p=esp32x->send_cmd;
	do
	{
		uint16_t singal_size=length<65535?length:65535;
		DMA1_Stream6->M0AR = (uint32_t)p;
    DMA1_Stream6->NDTR =singal_size;
		DMA_Cmd(DMA1_Stream6,ENABLE);
		xSemaphoreTake(semaphore_send,portMAX_DELAY);
		p+=singal_size;
		length-=singal_size;
	}while(length>0);
	return esp_basic_response(timerout);
}

static bool esp_wait_ready(esp32c3_handler espc32x,uint64_t timerout)
{
		memset(espc32x->recv_buf,0,espc32x->recv_len);
		return esp_basic_response(timerout)==AT_ACK_READY;
}

static bool esp32_reboot_init(esp32c3_handler esp32x,int64_t timerout)
{
		while(timerout>0)
		{
			timerout-=1000;
			if(esp_at_write_command(esp32x,"AT",1000))
			{
					return true;
			}
		}
		return false;
}

static void dma_lower_init(esp32c3_handler esp32x)
{
	DMA_InitTypeDef DMA_InStructer;
	DMA_StructInit(&DMA_InStructer);
	DMA_InStructer.DMA_Channel=DMA_Channel_4;
	DMA_InStructer.DMA_DIR=DMA_DIR_MemoryToPeripheral;
	DMA_InStructer.DMA_FIFOMode=DMA_FIFOMode_Enable;
	DMA_InStructer.DMA_FIFOThreshold=DMA_FIFOThreshold_Full;
	DMA_InStructer.DMA_MemoryBurst=DMA_MemoryBurst_INC8;
	DMA_InStructer.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_InStructer.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InStructer.DMA_Mode=DMA_Mode_Normal;
	DMA_InStructer.DMA_PeripheralBaseAddr=(uint32_t)&esp32x->usartx->usart_num->DR;
	DMA_InStructer.DMA_PeripheralBurst=DMA_PeripheralBurst_Single;
	DMA_InStructer.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_InStructer.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InStructer.DMA_Priority=DMA_Priority_Medium;
	DMA_Init(DMA1_Stream6,&DMA_InStructer);
}

static void dma_interrupt_init(esp32c3_handler esp32x)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitTypeDef NVIC_InStructer;
	memset(&NVIC_InStructer,0,sizeof(NVIC_InStructer));
	NVIC_InStructer.NVIC_IRQChannel=DMA1_Stream6_IRQn;
	NVIC_InStructer.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InStructer.NVIC_IRQChannelPreemptionPriority=5;
	NVIC_InStructer.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InStructer);
//	   NVIC_SetPriority(DMA1_Stream4_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_4, 15, 0));
//    // 使能中断
//    NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	//NVIC_SetPriority(DMA1_Stream6_IRQn, 5);
	USART_DMACmd(esp32x->usartx->usart_num,USART_DMAReq_Tx,ENABLE);
	DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,ENABLE);
}

static void dma_init(esp32c3_handler esp32x)
{
	dma_lower_init(esp32x);
	dma_interrupt_init(esp32x);

}


/**
 * @brief  ESP32C3初始化：串口+DMA+复位+AT测试
 * @param  esp32c3x  ESP32C3句柄
 * @retval true=成功
 */
bool esp32_init(esp32c3_handler esp32c3x)
{

	if(!esp32c3x)
	{
		return false;
	}
	taskENTER_CRITICAL();
	usart_init(esp32c3x->usartx);
	//queue_init(esp32c3x->recv_queue);
	dma_init(esp32c3x);
	taskEXIT_CRITICAL();
	#if DEBUG_ESP32
	uint32_t usart2_prio = NVIC_GetPriority(USART2_IRQn);
	uint32_t dma_prio = NVIC_GetPriority(DMA1_Stream6_IRQn);
	printf("USART2 IRQ prio reg: 0x%02X (logical %d)\r\n", (unsigned)usart2_prio, (unsigned)(usart2_prio >> 4));
	printf("DMA IRQ prio reg: 0x%02X (logical %d)\r\n", (unsigned)dma_prio, (unsigned)(dma_prio >> 4));
	#endif
		//防止上电后不稳定
	if(!esp32_reboot_init(esp32c3x,3000))
	{
		return false;
	}
	//printf("ESP init: first AT (ignore)\r\n");
	esp_at_write_command(esp32c3x,"AT",1000);
	 //printf("ESP init: second AT\r\n");
	//第一条指令用于清除不稳定状态的字符
			//queue_init(esp32c3x->recv_queue);
	//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	if(!esp_at_write_command(esp32c3x,"AT",2000))
	{

		//printf("ESP init: second AT failed\r\n");
		//queue_init(esp32c3x->recv_queue);
		//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
		return false;
	}
		//queue_init(esp32c3x->recv_queue);
	//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	if(!esp_at_write_command(esp32c3x,"AT+RESTORE",5000))
	{
		//复位
		//printf("ESP init: AT+RESTORE failed\r\n");
		//queue_init(esp32c3x->recv_queue);
		//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
		return false;
	}
			//queue_init(esp32c3x->recv_queue);
	//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	if(!esp_wait_ready(esp32c3x,5000))
	{
		//printf("ESP init: wait ready failed\r\n");
		//queue_init(esp32c3x->recv_queue);
		//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
		return false;
	}
	//printf("ESP init: success\r\n");
	//queue_init(esp32c3x->recv_queue);
	//memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	return true;
}

/**
 * @brief  发送AT命令并等待响应
 * @param  esp32c3x  ESP32C3句柄
 * @param  cmd       AT命令
 * @param  ack       期望响应
 * @param  timeout   超时ms
 * @retval true=收到ack
 */
bool esp_at_write_command(esp32c3_handler esp32c3x,const char*cmd,
	uint64_t timeout){
	if(!esp32c3x)
	{
		return false;
	}
	set_send_cmd(esp32c3x,cmd,strlen(cmd));
	xQueueReset(queue_recv);
	//m_queue_reset(esp32c3x->recv_queue);
	memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	at_ack_t ack=esp_write_by_usart(esp32c3x,timeout);
	if(ack==AT_ACK_OK)
	{
		#if DEBUG_ESP32
		printf("recv_buf:%s\r\n",esp32c3x->recv_buf);
		#endif
		return true;
	}
	#if DEBUG_ESP32
	printf("recv_buf:%s\r\n",esp32c3x->recv_buf);
	#endif
	xQueueReset(queue_recv);
	//m_queue_reset(esp32c3x->recv_queue);
	memset(esp32c3x->recv_buf,0,esp32c3x->recv_len);
	return false;
}

void esp_usart_recv_callback_register(esp32c3_handler esp32c3x
	,usart_callback_t func)
{
	if(esp32c3x&&func)
	{
		esp32c3x->usartx->callback_t=func;
	}
}

/**
 * @brief  WiFi初始化：设置Station模式+单连接
 * @param  esp32x  ESP32C3句柄
 * @retval true=成功
 */
bool esp_wifi_init(esp32c3_handler esp32x)
{
	//queue_init(esp32x->recv_queue);
	//memset(esp32x->recv_buf,0,esp32x->recv_len);
	return esp_at_write_command(esp32x,"AT+CWMODE=1",5000);
}

static bool set_esp_wifi_info(esp32c3_handler esp32x
	,const char*ssid,const char*pwd,const char*mac)
{
	if(ssid&&pwd)
	{
		strncpy(esp32x->esp_wifi_info_pointer->ssid,\
		ssid,esp32x->esp_wifi_info_pointer->ssid_len);
		strncpy(esp32x->esp_wifi_info_pointer->pwd
		,pwd,esp32x->esp_wifi_info_pointer->pwd_len);
	}
	else 
	{
		return false;
	}
	if(mac)
	{
		strncpy(esp32x->esp_wifi_info_pointer->bssid,
		mac,esp32x->esp_wifi_info_pointer->bssid_len);
	}
	return true;
}

/**
 * @brief  连接指定WiFi热点
 * @param  esp32x  ESP32C3句柄
 * @param  ssid     SSID
 * @param  pwd      密码
 * @param  timeout  超时ms
 * @retval true=连接成功
 */
bool esp_wifi_connect(esp32c3_handler esp32x\
	,const char*ssid,const char*pwd,const char*mac,uint64_t timeout)
{
		if(!esp32x)
		{
			return false;
		}
		if(!set_esp_wifi_info(esp32x,ssid,pwd,mac))
		{
			//queue_init(esp32x->recv_queue);
			//memset(esp32x->recv_buf,0,esp32x->recv_len);
			return false;
		}
		char buf[64]={0};
		uint8_t len=snprintf(buf,sizeof(buf)/sizeof(char),\
			"AT+CWJAP=\"%s\",\"%s\"",\
		esp32x->esp_wifi_info_pointer->ssid
		,esp32x->esp_wifi_info_pointer->pwd);
		return esp_at_write_command(esp32x,buf,timeout);
}

static bool esp_parse_cwstate_response(esp32c3_handler esp32x)
{
	//printf("%s full_recv_buf:%s\r\n",__FUNCTION__,esp32x->recv_buf);
//	printf("%s full_recv_buf:%s\r\n",__FUNCTION__,esp32x->recv_buf);

	char *p=strstr(esp32x->recv_buf,"+CWSTATE:");
	if(!p)
	{
		return false;
	}
//		printf("p hex: ");
//	for (int i=0; i<20 && p[i]; i++) printf("%02x ", p[i]);
//	printf("\r\n");
		uint8_t state=0;
		uint8_t ret_val=sscanf(p,"+CWSTATE:%hhu,\"%63[^\"]\"",
	&state,esp32x->esp_wifi_info_pointer->ssid);
	//printf("%s %d %s",__FUNCTION__,state,esp32x->esp_wifi_info_pointer->ssid);
	if(ret_val!=2)
	{
//		printf("%s ret_val:%d\r\n\r\n",__FUNCTION__,ret_val);
		return false;
	}
	if(state!=2)
	{
//		printf("%s state:%d",__FUNCTION__,state);
		return false;
	}
	esp32x->esp_wifi_info_pointer->connected=true;
	return true;
}

static bool esp_parse_cwjap_response(esp32c3_handler esp32x)
{
	#if DEBUG_ESP32
	printf("%s recv_buf:%s\r\n",__FUNCTION__,esp32x->recv_buf);
	#endif
	char *p=strstr(esp32x->recv_buf,"+CWJAP:");
	if(!p)
	{
		return false;
	}
	uint8_t ret_val=sscanf(p,"+CWJAP:\"%63[^\"]\",\"%17[^\"]\",%d,%d",
	esp32x->esp_wifi_info_pointer->ssid,esp32x->esp_wifi_info_pointer->bssid,
	&esp32x->esp_wifi_info_pointer->channal,&esp32x->esp_wifi_info_pointer->rssi);
	if(ret_val!=4)
	{
		return false;
	}
	return true;
}

/**
 * @brief  获取WiFi信息（IP等）
 * @param  esp32x  ESP32C3句柄
 * @retval true=成功
 */
bool esp_at_get_wifi_info(esp32c3_handler esp32x)
{
	if(!esp32x)
	{
		
		return false;
	}
	if(!esp_at_write_command(esp32x,"AT+CWSTATE?",3000))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		esp32x->esp_wifi_info_pointer->connected = false;
		return false;
	}

	if(!esp_parse_cwstate_response(esp32x))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		esp32x->esp_wifi_info_pointer->connected = false;
		return false;
	}
		if(esp32x->esp_wifi_info_pointer->connected)
	{
	if(!esp_at_write_command(esp32x,"AT+CWJAP?",3000))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		esp32x->esp_wifi_info_pointer->connected = false;
		return false;
	}
	if(!esp_parse_cwjap_response(esp32x))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		esp32x->esp_wifi_info_pointer->connected = false;
		return false;
	}
	}
	esp32x->esp_wifi_info_pointer->connected = true;
	return true;
}

/**
 * @brief  SNTP初始化：配置NTP服务器
 * @param  esp32x  ESP32C3句柄
 * @retval true=成功
 */
bool esp_at_sntp_init(esp32c3_handler esp32x)
{
	if(!esp32x)
	{
		return false;
	}
	if(!esp_at_write_command(esp32x,"AT+CIPSNTPCFG=1,8",5000))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		return false;
	}
	return true;
}

static uint8_t month_str_to_num(const char *month_str)
{
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (uint8_t i = 0; i < 12; i++)
	{
		if (strcmp(month_str, months[i]) == 0)
		{
			return i + 1;
		}
	}
	return 0;
}


static uint8_t weekday_str_to_num(const char *weekday_str)
{
	const char *weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	for (uint8_t i = 0; i < 7; i++) {
		if (strcmp(weekday_str, weekdays[i]) == 0)
		{
			return i + 1;
		}
	}
	return 0;
}

static bool esp_parse_cipsntptime_response(esp32c3_handler esp32x)
{
	printf("%s %s\r\n\r\n",__FUNCTION__,esp32x->recv_buf);
	char *p=strstr(esp32x->recv_buf,"+CIPSNTPTIME:");
	if(p==NULL)
	{
		return false;
	}
	char month[4]={0};
	char weekday[4]={0};
	uint8_t ret_val=sscanf(p,
	"+CIPSNTPTIME:%3s %3s %hhu %hhu:%hhu:%hhu %hu",
	weekday,month,&esp32x->esp_data_struct_pointer->day,
	&esp32x->esp_data_struct_pointer->hour,&esp32x->esp_data_struct_pointer->minute,
	&esp32x->esp_data_struct_pointer->second,&esp32x->esp_data_struct_pointer->year);
	if(ret_val!=7)
	{
		return false;
	}
	esp32x->esp_data_struct_pointer->weekday=weekday_str_to_num(weekday);
	esp32x->esp_data_struct_pointer->month=month_str_to_num(month);
	return true;
}

/**
 * @brief  从SNTP获取时间
 * @param  esp32x  ESP32C3句柄
 * @retval true=成功
 */
bool esp_sntp_get_time(esp32c3_handler esp32x)
{
	if(!esp32x)
	{
		return false;
	}
	if(!esp_at_write_command(esp32x,"AT+CIPSNTPTIME?",3000))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
			return false;
	}
	if(!esp_parse_cipsntptime_response(esp32x))
	{
		//queue_init(esp32x->recv_queue);
		//memset(esp32x->recv_buf,0,esp32x->recv_len);
		return false;
	}
	return true;
}

/**
 * @brief  发送HTTP GET请求
 * @param  esp32x  ESP32C3句柄
 * @param  url      请求URL
 * @retval true=成功
 */
bool esp_http_get(esp32c3_handler esp32x,const char*url)
{
	if(!esp32x||!url)
	{
		return false;
	}
	static char buf[512]={0};
	snprintf(buf,sizeof(buf)/sizeof(char),"AT+HTTPCLIENT=2,1,\"%s\",,,2",url);
	return esp_at_write_command(esp32x,buf,5000);
}

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE))
	{
//		if(USART_desc_2->callback_t)
//		{
//			uint8_t data=USART_ReceiveData(USART2);
//		}
		//printf("enter recv\r\n");
		uint8_t data=USART_ReceiveData(USART2);
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendFromISR(queue_recv,&data,&pxHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
}

void DMA1_Stream6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream6,DMA_IT_TCIF6)==SET)
	{
		//printf("enter\r\n");
		BaseType_t pxHigherPriorityTaskWoken;
		xSemaphoreGiveFromISR(semaphore_send,&pxHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		DMA_ClearITPendingBit(DMA1_Stream6,DMA_IT_TCIF6);
	}
}


// ============ WiFi getter ============
const char* esp_get_wifi_ssid(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_wifi_info_pointer) return NULL;
    return esp32x->esp_wifi_info_pointer->ssid;
}

const char* esp_get_wifi_bssid(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_wifi_info_pointer) return NULL;
    return esp32x->esp_wifi_info_pointer->bssid;
}

int32_t esp_get_wifi_rssi(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_wifi_info_pointer) return -999;
    return esp32x->esp_wifi_info_pointer->rssi;
}

int32_t esp_get_wifi_channel(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_wifi_info_pointer) return -1;
    return esp32x->esp_wifi_info_pointer->channal;
}

/**
 * @brief  查询WiFi是否已连接
 * @param  esp32x  ESP32C3句柄
 * @retval true=已连接
 */
bool esp_get_wifi_connected(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_wifi_info_pointer) return false;
    return esp32x->esp_wifi_info_pointer->connected;
}

// ============ SNTP time getter ============
/**
 * @brief  获取年份
 * @param  esp32x  ESP32C3句柄
 * @retval 年份
 */
uint16_t esp_get_time_year(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->year;
}

/**
 * @brief  获取月份
 * @param  esp32x  ESP32C3句柄
 * @retval 月份
 */
uint8_t esp_get_time_month(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->month;
}

/**
 * @brief  获取日
 * @param  esp32x  ESP32C3句柄
 * @retval 日
 */
uint8_t esp_get_time_day(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->day;
}

/**
 * @brief  获取小时
 * @param  esp32x  ESP32C3句柄
 * @retval 小时
 */
uint8_t esp_get_time_hour(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->hour;
}

/**
 * @brief  获取分钟
 * @param  esp32x  ESP32C3句柄
 * @retval 分钟
 */
uint8_t esp_get_time_minute(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->minute;
}

/**
 * @brief  获取秒
 * @param  esp32x  ESP32C3句柄
 * @retval 秒
 */
uint8_t esp_get_time_second(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->second;
}

/**
 * @brief  获取星期
 * @param  esp32x  ESP32C3句柄
 * @retval 星期
 */
uint8_t esp_get_time_weekday(const esp32c3_handler esp32x)
{
    if (!esp32x || !esp32x->esp_data_struct_pointer) return 0;
    return esp32x->esp_data_struct_pointer->weekday;
}
