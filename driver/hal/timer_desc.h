#ifndef __TIME_DESC_H_
#define __TIME_DESC_H_
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
typedef void(*tim_callback_func)(void);
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
typedef void (*timer_callback_t)(USART_MY_handle);
typedef struct timer_struct* timer_handler;
typedef void (*timer_ic_callback_t)(timer_handler,USART_MY_handle);
struct timer_base
{
		TIM_TypeDef* timx;//定时器编号
		uint16_t timer_presc;//定时器分频系数
		uint16_t timer_count_mode;//定时器计数模式
		uint32_t timer_period;//定时器周期（arr寄存器的值）
		uint16_t timer_div;//时钟分频（滤波时钟）
		uint8_t rcr_value;//rcr寄存器的值
		timer_callback_t callback;//回调函数
};
typedef struct timer_base* timer_base_handle;
struct timer_intputCaputer{
	uint16_t tim_channel1;//捕获通道
	uint16_t tim_polarity1;//触发边沿
	uint16_t tim_selection1;//直接或者相反输入
	uint16_t prescaler1;//通道分频
	uint16_t filter1;//滤波系数
	uint16_t tim_channel2;//捕获通道
	uint16_t tim_polarity2;//触发边沿
	uint16_t tim_selection2;//直接或者相反输入
	uint16_t prescaler2;//通道分频
	uint16_t filter2;//滤波系数
	GPIO_TypeDef * GPIO_PORT1;//端口
	uint16_t GPIO_PIN1;//引脚
	GPIO_TypeDef *GPIO_PORT2;
	uint16_t GPIO_PIN2;
	//uint16_t GPIO_port_source1;
	uint16_t GPIO_pin_source1;
	//uint16_t GPIO_port_source2;
	uint16_t GPIO_pin_source2;
	uint16_t slave_mode;//从模式选择
	uint16_t trigger_source;//触发源
	timer_ic_callback_t callback;//回调函数
};
typedef struct timer_intputCaputer* timer_inputCaputer_handle;
struct timer_outputCompare{
		uint8_t channal;//输出通道
		uint16_t out_put_mode;//PWM输出的模式
		uint16_t out_put_state;//主通道使能
		uint16_t out_put_nstate;//互补通道使能
		uint32_t ccr;//ccr寄存器的值
		uint16_t ocpolarity;//主通道极性
		uint16_t ocnpolarity;//互补通道极性
		uint16_t ocidlestate;//主通道空闲时电平
		uint16_t ocnidlestate;//互补通道空闲时电平
		GPIO_TypeDef * GPIO_PORT;//端口
		uint16_t GPIO_PIN;//引脚
		bool isfast;//是否开高速模式 
		//uint16_t GPIO_port_source;
		uint16_t GPIO_pin_source;
};
typedef struct timer_outputCompare* timer_outputCompare_handle;
struct timer_struct
{
		timer_base_handle base;
		timer_inputCaputer_handle intput_Caputer;
		timer_outputCompare_handle timer_output_Compare;
		bool isInterrupt;//是否开启中断
		bool isOC;//是否开启输出比较
		bool isIC;//是否开启输入捕获

		uint8_t irqn;//中断通道
		uint16_t IT;//使能什么类型的中断
};

#endif
