/**
 ******************************************************************************
 * @file    board.c
 * @brief   STM32F4 板级初始化：GPIO/USART/SPI/I2C/RTC/LCD/ESP32C3 引脚配置
 ******************************************************************************
 */

#include "stm32f4xx.h"
//#include "led_desc.h"
//#include "led.h"
#include "usart_desc.h"
#include "usart.h"
//#include "Key.h"
//#include "Key_desc.h"
#include <stdio.h>
#include "timer.h"
#include "timer_desc.h"
//#include "mpu6050.h"
//#include "mpu6050_desc.h"
//#include "i2c.h"
//#include "i2c_desc.h"
#include "dht11.h"
#include "dht11_desc.h"
#include "spi.h"
#include "spi_desc.h"
#include "lcd24_desc.h"
#include "lcd24.h"
#include "front.h"
#include "front_desc.h"
#include "image.h"
#include "image_desc.h"
#include "esp32c3.h"
#include "esp32c3_desc.h"
#include "weather.h"
#include "weather_desc.h"
#include "rtc.h"
#include "rtc_desc.h"
#include "gpio.h"
#include "flex_desc.h"
#include "flex_key.h"
struct m_queue;
typedef  struct m_queue* queue_handle;
//#define LED_DESC_(x,PORT,PIN,ON,OFF)\
//static struct led_desc __led##x={\
//GPIO##PORT,GPIO_Pin_##PIN,ON,OFF\
//};
//LED_DESC_(1,E,9,Bit_SET,Bit_RESET)
//LED_DESC_(2,B,0,Bit_SET,Bit_RESET)
//LED_DESC_(3,B,1,Bit_SET,Bit_RESET)
////加static是不将这几个对象暴露给外部
////static struct led_desc __led1={GPIOE,GPIO_Pin_9,\
////Bit_SET,Bit_RESET};
////static struct led_desc __led2={GPIOB,GPIO_Pin_0,\
////Bit_SET,Bit_RESET};
////static struct led_desc __led3={GPIOB,GPIO_Pin_1,\
////Bit_SET,Bit_RESET
////};

////只将指针暴露给外部
//led_desc_t led1=&__led1;
//led_desc_t led2=&__led2;
//led_desc_t led3=&__led3;

#define USART_DESC(x,PORT,PIN1,PIN2,speed,TX_INTERRUPT,RX_INTERRUPT,IRQN)\
static struct USART_desc __USART##x={\
GPIO_PinSource##PIN1,GPIO_PinSource##PIN2,USART##x,speed,\
GPIO##PORT,GPIO_Pin_##PIN1,GPIO_Pin_##PIN2,TX_INTERRUPT,RX_INTERRUPT,NULL,IRQN\
};

USART_DESC(1,A,9,10,115200,RESET,SET,USART1_IRQn)

//static struct USART_desc __USART1={GPIO_PinSource9,\
//	GPIO_PinSource10,USART1,115200,GPIOA,GPIO_Pin_9,GPIO_Pin_10,\
//	RESET,SET,NULL
//};

USART_MY_handle USART_desc_1=&__USART1;

// 转换宏:根据exti_line自动匹配IRQn
#define GET_EXTI_IRQ(line) ( \
    (line == 0) ? EXTI0_IRQn : \
    (line == 1) ? EXTI1_IRQn : \
    (line == 2) ? EXTI2_IRQn : \
    (line == 3) ? EXTI3_IRQn : \
    (line == 4) ? EXTI4_IRQn : \
    ((line >= 5) && (line <= 9)) ? EXTI9_5_IRQn : \
    EXTI15_10_IRQn \
)

//#define KEY_STRUCT(x,PORT,PIN,ON,OFF,ISEXIT,LINE,TRIGGER)\
//static struct key_struct __key##x={\
//GPIO##PORT,GPIO_Pin_##PIN,ON,OFF,ISEXIT,EXTI_Line##LINE,\
//EXTI_PortSourceGPIO##PORT,EXTI_PinSource##PIN,TRIGGER,GET_EXTI_IRQ(LINE),NULL\
//};
//KEY_STRUCT(1,A,0,Bit_RESET,Bit_SET,true,0,Rising)
//KEY_STRUCT(2,C,4,Bit_RESET,Bit_SET,true,4,Rising)
////static struct key_struct __key1={GPIOA,GPIO_Pin_0,Bit_RESET,Bit_SET,true,EXTI_Line0,
////EXTI_PortSourceGPIOA,EXTI_PinSource0,Rising,EXTI0_IRQn,NULL};
////static struct key_struct __key2={GPIOC,GPIO_Pin_4,Bit_RESET,Bit_SET,true,EXTI_Line4,
////EXTI_PortSourceGPIOC,EXTI_PinSource4,Rising,EXTI4_IRQn,NULL};

//key_handle key1=&__key1;
//key_handle key2=&__key2;





// 1. 基础定时器宏（生成一个独立的 timer_struct）
#define TIMER_BASE_DESC(name, TIMx, PRESC, MODE, PERIOD, DIV, RCR, IRQ_NUM, IT_TYPE) \
static struct timer_base __base_##name = { \
    TIMx, PRESC, MODE, PERIOD, DIV, RCR,NULL \
}; \
static struct timer_struct name = { \
    .base = &__base_##name, \
    .intput_Caputer = NULL, \
    .timer_output_Compare = NULL, \
    .isInterrupt = (IT_TYPE != 0), \
    .isOC = false, \
    .isIC = false, \
    .irqn = IRQ_NUM, \
    .IT = IT_TYPE \
}

// 2. 输入捕获宏（生成一个独立的 timer_struct）
#define TIMER_IC_DESC(name, TIMx, PRESC, MODE, PERIOD, DIV, RCR, \
                       CH1, POL1, SEL1, PSC1, FILT1, \
											 CH2, POL2, SEL2, PSC2, FILT2, \
                       PORT1, PIN1, PORT2, PIN2, \
                       IRQ_NUM, IT_TYPE,slave_mode,trigger_source) \
static struct timer_base __base_##name = { \
    TIMx, PRESC, MODE, PERIOD, DIV, RCR,NULL\
}; \
static struct timer_intputCaputer __ic_##name = { \
    CH1, POL1, SEL1, PSC1, FILT1, \
		CH2, POL2, SEL2, PSC2, FILT2, \
    GPIO##PORT1, GPIO_Pin_##PIN1, GPIO##PORT2, GPIO_Pin_##PIN2, \
    GPIO_PinSource##PIN1,GPIO_PinSource##PIN2,slave_mode,trigger_source,NULL \
}; \
static struct timer_struct name = { \
    .base = &__base_##name, \
    .intput_Caputer = &__ic_##name, \
    .timer_output_Compare = NULL, \
    .isInterrupt = true, \
    .isOC = false, \
    .isIC = true, \
    .irqn = IRQ_NUM, \
    .IT = IT_TYPE \
}

// 3. 输出比较宏（生成一个独立的 timer_struct）
#define TIMER_OC_DESC(name, TIMx, PRESC, MODE, PERIOD, DIV, RCR, \
                       CH, OCM, OSTATE, ONSTATE, CCR, OCP, OCNP, OCID, OCNID, \
                       PORT, PIN, ISFAST, \
                       IRQ_NUM, IT_TYPE) \
static struct timer_base __base_##name = { \
    TIMx, PRESC, MODE, PERIOD, DIV, RCR ,NULL\
}; \
static struct timer_outputCompare __oc_##name = { \
    CH, OCM, OSTATE, ONSTATE, CCR, OCP, OCNP, OCID, OCNID, \
    GPIO##PORT, GPIO_Pin_##PIN, ISFAST,GPIO_PinSource##PIN \
}; \
static struct timer_struct name = { \
    .base = &__base_##name, \
    .intput_Caputer = NULL, \
    .timer_output_Compare = &__oc_##name, \
    .isInterrupt = (IT_TYPE != 0), \
    .isOC = true, \
    .isIC = false, \
    .irqn = IRQ_NUM, \
    .IT = IT_TYPE \
}


TIMER_BASE_DESC(T6_Basic, TIM6, 84-1, TIM_CounterMode_Up, 1000-1, TIM_CKD_DIV1, 0, TIM6_DAC_IRQn, TIM_IT_Update);
//TIMER_BASE_DESC(T6_Basic, TIM6, 84-1, TIM_CounterMode_Up, 1000-1, TIM_CKD_DIV1, 0, TIM6_DAC_IRQn, 0);
timer_handler timer_handler_1=&T6_Basic;

//TIMER_OC_DESC(T3_PWM, TIM3, 84-1, TIM_CounterMode_Up, 10000-1, TIM_CKD_DIV1, 0, \
//              1, TIM_OCMode_PWM1, TIM_OutputState_Enable, TIM_OutputNState_Disable, 0, \
//              TIM_OCPolarity_High, TIM_OCNPolarity_High, TIM_OCIdleState_Reset, TIM_OCIdleState_Reset, \
//              A, 6, false, \
//              0, 0);
//timer_handler timer_handler_2=&T3_PWM;
//							
//TIMER_IC_DESC(T2_IC, TIM4, 84-1, TIM_CounterMode_Up, 0xFFFFFFFF, TIM_CKD_DIV1, 0, \
//              TIM_Channel_1, TIM_ICPolarity_Rising, TIM_ICSelection_DirectTI, TIM_ICPSC_DIV1, 0x0F, \
//              TIM_Channel_2, TIM_ICPolarity_Falling, TIM_ICSelection_IndirectTI, TIM_ICPSC_DIV1, 0x0F, \
//							B, 6, B, 7,\
//              TIM4_IRQn, TIM_IT_CC1,TIM_SlaveMode_Reset,TIM_TS_TI1FP1);
//timer_handler timer_handler_3=&T2_IC;
							
							
//#define I2C_DESC(x, I2Cx, SCL_PORT, SCL_PIN,SDA_PORT, SDA_PIN, \
//                 AF, CLKSPEED, MODE, DUTY, OWN, ACK, ADDWIDTH) \
//static struct my_i2c __i2c##x = { \
//    I2C##x,                /* i2cx */ \
//    (CLKSPEED),            /* clockspeed */ \
//    (MODE),                /* mode */ \
//    (DUTY),                /* duty */ \
//    (OWN),                 /* ownaddress */ \
//    (ACK),                 /* isack */ \
//    (ADDWIDTH),            /* widthofaddress */ \
//    GPIO##SCL_PORT,        /* GPIO_Port_scl */ \
//    GPIO_Pin_##SCL_PIN,    /* GPIO_Pin_scl */ \
//    GPIO##SDA_PORT,        /* GPIO_Port_sda */ \
//    GPIO_Pin_##SDA_PIN,    /* GPIO_Pin_sda */ \
//    GPIO_PinSource##SCL_PIN, /* GPIO_pinSource_scl */ \
//    GPIO_PinSource##SDA_PIN, /* GPIO_pinSource_sda */ \
//    (AF)                   /* GPIO_af */ \
//}; \

//I2C_DESC(1, I2C1, B, 6, B, 7,GPIO_AF_I2C1,
//         400*1000, I2C_Mode_I2C, I2C_DutyCycle_2,
//         0x00, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);
//i2c_handle i2c1=&__i2c1;

//#define MPU6050_DESC(x,DEFAULT)\
//static struct REG __reg##x=\
//{\
//DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT,DEFAULT\
//};\

//MPU6050_DESC(1,0);
//mpu6050_handler mpu6050_1=&__reg1;

#define dht11_DESC(x,PORT,PIN)\
static struct dht11_struct dht11_##x={\
GPIO##PORT,GPIO_Pin_##PIN\
};

dht11_DESC(1,A,4);
dth_handle dht11_1_handler =&dht11_1;

#define SPI_DESC(x, SPIx, SCK_PORT, SCK_PIN, MISO_PORT, MISO_PIN, MOSI_PORT, MOSI_PIN,CS_PORT, CS_PIN) \
static struct spi_Struct __spi##x = { \
    .spinum = SPIx, \
    .GPIO_SCK_PORT = GPIO##SCK_PORT, \
    .GPIO_SCK_PIN = GPIO_Pin_##SCK_PIN, \
    .GPIO_SCK_PIN_source = GPIO_PinSource##SCK_PIN, \
    .GPIO_MISO_PORT = GPIO##MISO_PORT, \
    .GPIO_MISO_PIN = GPIO_Pin_##MISO_PIN, \
    .GPIO_MISO_PIN_source = GPIO_PinSource##MISO_PIN, \
    .GPIO_MOSI_PORT = GPIO##MOSI_PORT, \
    .GPIO_MOSI_PIN = GPIO_Pin_##MOSI_PIN, \
    .GPIO_MOSI_PIN_source = GPIO_PinSource##MOSI_PIN, \
		.GPIO_CS_PORT=GPIO##CS_PORT,\
		.GPIO_CS_PIN=GPIO_Pin_##CS_PIN\
}; \

SPI_DESC(1,SPI2,B,13,C,2,C,3,E,2);
spi_handler spi_handler1=&__spi1;

#define LCD24_DESC(x,SPI_HANDLE,RESET_PORT, RESET_PIN, DC_PORT, DC_PIN) \
static struct lcd24 __lcd24##x = { \
    .spi_x = (SPI_HANDLE), \
    .GPIO_RESET_PORT = GPIO##RESET_PORT, \
    .GPIO_RESET_PIN = GPIO_Pin_##RESET_PIN, \
    .GPIO_DC_PORT = GPIO##DC_PORT, \
    .GPIO_DC_PIN = GPIO_Pin_##DC_PIN, \
}; \

LCD24_DESC(1,&__spi1,E,3,E,4);
lcd24_handler lcd241=&__lcd241;

#define FRONT_DESC(HEIGHT,WEIGHT,ARR_ADDRESS,MAP_ADDRESS,CN,SIZE)\
static struct front_struct __front##HEIGHT##X##WEIGHT={\
HEIGHT,WEIGHT,(ARR_ADDRESS),(CN),SIZE,MAP_ADDRESS\
};

//FRONT_DESC(16,16,front_data_16,NULL,0);
//front_handler front_16X16=&__front16X16;
FRONT_DESC(24,24,front_data_24,NULL,NULL,0);
front_handler front_24X24=&__front24X24;

FRONT_DESC(32,32,front_data_32,NULL,front_data32X32_CH,8);
front_handler front_32X32=&__front32X32;

FRONT_DESC(20,20,front_data_20,NULL,chinese_fonts_20,11);
front_handler front_20X20=&__front20X20;

FRONT_DESC(16,16,ascii_model_16,NULL,chinese_fonts_ext_16,4);
front_handler front16X16=&__front16X16;

FRONT_DESC(76,76,ascii_model_76,(const uint8_t*)"0123456789: -",NULL,0);
front_handler front76X76=&__front76X76;

FRONT_DESC(54,54,ascii_model_54,(const uint8_t*)"-0123456789. ",NULL,0);
front_handler front_54X54=&__front54X54;

FRONT_DESC(64,64,ascii_model_64,(const uint8_t*)"-0123456789. ",NULL,0);
front_handler front_64X64=&__front64X64;

#define IMAGE_DESC(x,HEIGHT,WEIGHT,ADDR_ARR)\
static struct image_struct __image##x={\
HEIGHT,WEIGHT,(ADDR_ARR)\
};

IMAGE_DESC(1,240,118,gImage_image_data);
image_handle image1=&__image1;

//static struct image_struct image_heiqiyihu_welcome={
//	.height=180,
//	.weight=180,
//	.pdata_arr=gImage_image_heiqiyihu
//};
IMAGE_DESC(2,180,180,gImage_image_heiqiyihu)

image_handle image_heiqiyihu_welcome_handle=&__image2;


//static struct image_struct image_zuozhu_error={
//	.height=160,
//	.weight=160,
//	.pdata_arr=gImage_image_zuozhu
//};
IMAGE_DESC(3,160,160,gImage_image_zuozhu);

image_handle image_zuozhu_error_handle=&__image3;

//static struct image_struct image_sihuang_wifi={
//	.height=164,
//	.weight=180,
//	.pdata_arr=gImage_image_sihuang
//};
IMAGE_DESC(4,164,180,gImage_image_sihuang);

image_handle image_sihuang_wifi_handle=&__image4;

//static struct image_struct image_wifi={
//	.weight=24,
//	.height=24,
//	.pdata_arr=gImage_icon_wifi
//};
IMAGE_DESC(5,24,24,gImage_icon_wifi);
image_handle image_wifi_icon=&__image5;

IMAGE_DESC(6,51,21,gImage_icon_wenduji);
image_handle image_wenduji_icon=&__image6;

IMAGE_DESC(7,54,54,gImage_icon_qing);
image_handle image_qing_icon=&__image7;

IMAGE_DESC(8,54,54,gImage_icon_leizhenyu);
image_handle image_leizhenyu=&__image8;

IMAGE_DESC(9,54,54,gImage_icon_na);
image_handle image_na=&__image9;

IMAGE_DESC(10,54,54,gImage_icon_yintian);
image_handle image_yintian=&__image10;

IMAGE_DESC(11,54,54,gImage_icon_yueliang);
image_handle image_yueliang=&__image11;

IMAGE_DESC(12,54,54,gImage_icon_zhongxue);
image_handle image_zhongxue=&__image12;

IMAGE_DESC(13,54,54,gImage_icon_zhongyu);
image_handle image_zhongyu=&__image13;

IMAGE_DESC(14,54,54,gImage_icon_duoyun);
image_handle image_duoyun=&__image14;

#define TX_BUF_SIZE 512   /**< ESP32C3串口发送缓冲大小 */
#define RX_BUF_SIZE 512   /**< ESP32C3串口接收缓冲大小 */

static char __esp32c3_txbuf_1[TX_BUF_SIZE]={0};
static char __esp32c3_rxbuf_1[RX_BUF_SIZE]={0};
#define ESP32C3_DESC(x, USART_HANDLE, TX_SIZE, RX_SIZE) \
    static char __esp32c3_txbuf_##x[TX_SIZE]; \
    static char __esp32c3_rxbuf_##x[RX_SIZE]; \
    static struct esp32_c3 __esp32c3_##x = { \
        .usartx = (USART_HANDLE), \
        .send_cmd = __esp32c3_txbuf_##x, \
        .send_len = TX_SIZE, \
        .recv_buf = __esp32c3_rxbuf_##x, \
        .recv_len = RX_SIZE, \
				.esp_wifi_info_pointer=NULL,\
				.esp_data_struct_pointer=NULL,\
    };
		
#define ssid_len 32    /**< WiFi SSID最大长度 */
#define bssid_len 64   /**< WiFi BSSID最大长度 */
#define pwd_len 32     /**< WiFi密码最大长度 */		
char _ssid[ssid_len]={0};
char _bssid[bssid_len]={0};
char _pwd[pwd_len]={0};
		
#define ESP_WIFI_INFO(x,ssid_pointer,\
_ssid_len,bssid_pointer,_bssid_len,pwd_pointer,_pwd_len) \
static struct esp_wifi_info __esp_wifi_info##x = { \
(ssid_pointer),_ssid_len,(bssid_pointer),_bssid_len,0,0,(pwd_pointer),_pwd_len,false\
};
		

#define ESP_DATA(x)\
static struct esp_data_struct __esp_data_struct##x={0};

#define WEATHER(x)\
static struct weather __weather##x={0};

ESP_DATA(1);
ESP_WIFI_INFO(1,_ssid,ssid_len,_bssid,bssid_len,_pwd,pwd_len);
WEATHER(1);
weather_handler weather_1=&__weather1;
USART_DESC(2,A,2,3,115200,RESET,SET,USART2_IRQn)
USART_MY_handle USART_desc_2=&__USART2;
ESP32C3_DESC(1,&__USART2,TX_BUF_SIZE,RX_BUF_SIZE);
esp32c3_handler esp32c3_1=&__esp32c3_1;	

#define RTC_DESC(x)\
static struct rtc_struct __rtc##x={2026,7,29,20,40,9,3};
RTC_DESC(1);
rtc_handler rtc_handler_1=&__rtc1;

//#define KEY1_PORT A
//#define KEY1_PIN 0
#define KEY1_ON_LEVEL Bit_RESET
#define KEY1_OFF_LEVEL Bit_SET
#define GPIO_DESC(id,PORT,PIN,ON,OFF)\
static GPIO_T __gpio##id={\
	.GPIO_PORT=GPIO##PORT,\
	.GPIO_PIN=GPIO_Pin_##PIN,\
	.on=(ON),\
	.off=(OFF)\
};

GPIO_DESC(1,A,0,KEY1_ON_LEVEL,KEY1_OFF_LEVEL);

/* ---------- 时间参数（毫秒） ---------- */
#define FLEX_KEY_DEBOUNCE_MS        40    /* 消抖时间 */
#define FLEX_KEY_SHORT_MS           500   /* 短按阈值 */
#define FLEX_KEY_LONG_MS            1500  /* 长按阈值 */
#define FLEX_KEY_LONG_HOLD_MS       3000  /* 长保持阈值 */
#define FLEX_KEY_MULTI_INTERVAL_MS  300   /* 连击间隔 */

#define FLEX_KEY_TABLE(x,KEY_PRESS_LEVEL)\
static struct  flex_button __flex_button_##x ={\
        .next              = NULL,\
        .flex_usr_read     = NULL,\
        .cb                = NULL,\
        .press_level       = KEY_PRESS_LEVEL,\
        .event             = FLEX_BTN_PRESS_NONE,\
        .status            = FLEX_BTN_STAGE_DEFAULT,\
        .id                = x,\
        .scan_cnt          = 0,\
        .click_cnt         = 0,\
        .max_multiple_clicks_interval = FlEX_KEY_MS_TO_SCANTICK(FLEX_KEY_MULTI_INTERVAL_MS),\
        .debounce_tick     = FlEX_KEY_MS_TO_SCANTICK(FLEX_KEY_DEBOUNCE_MS),\
        .short_press_tick  = FlEX_KEY_MS_TO_SCANTICK(FLEX_KEY_SHORT_MS),\
        .long_press_tick   = FlEX_KEY_MS_TO_SCANTICK(FLEX_KEY_LONG_MS),\
        .long_hold_tick    = FlEX_KEY_MS_TO_SCANTICK(FLEX_KEY_LONG_HOLD_MS),\
				.gpio							 = NULL\
}; 

//int test_val = FlEX_KEY_MS_TO_SCANTICK(300);
//FLEX_KEY_TABLE(1,KEY1_ON_LEVEL);
FLEX_KEY_TABLE(1,KEY1_ON_LEVEL);
flex_button_handler flex_handler_1=&__flex_button_1;

//static struct flex_button __flex_button_1 = {
//    .next              = NULL,
//    .flex_usr_read     = NULL,
//    .cb                = NULL,
//    .press_level       = 0,
//    .event             = FLEX_BTN_PRESS_NONE,
//    .status            = FLEX_BTN_STAGE_DEFAULT,
//    .id                = 1,
//    .scan_cnt          = 0,
//    .click_cnt         = 0,
//    .max_multiple_clicks_interval = 15,
//    .debounce_tick     = 2,
//    .short_press_tick  = 25,
//    .long_press_tick   = 75,
//    .long_hold_tick    = 150,
//    .gpio              = NULL
//};


void board_init(void)
{
	//开启时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//外部中断要开
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	PWR_BackupAccessCmd(ENABLE);
  RCC_LSEConfig(RCC_LSE_ON);
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	__esp32c3_1.esp_data_struct_pointer=&__esp_data_struct1;
	__esp32c3_1.esp_wifi_info_pointer=&__esp_wifi_info1;
	flex_handler_1->gpio=&__gpio1;
	flex_handler_1->flex_usr_read=usr_key_read;
}
