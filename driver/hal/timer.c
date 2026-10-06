/**
 ******************************************************************************
 * @file    timer.c
 * @brief   定时器中断：LCD 背光、按键扫描时基
 ******************************************************************************
 */

#include "stm32f4xx.h"
#include <stdbool.h>
#include "timer.h"
#include "timer_desc.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "usart_desc.h"
//volatile uint32_t tick=0;
volatile static uint64_t tick=0;
#define TICKS_PER_MS (1000)
#define TICKS_PER_US (1)
//typedef void(*tim_callback_func)(void);
static tim_callback_func func_callback=NULL;
extern USART_MY_handle USART_desc_1;
extern timer_handler timer_handler_1;
//extern timer_handler timer_handler_3;
void m_time_init(timer_handler timerx){
	//时基单元初始化
	TIM_TimeBaseInitTypeDef timer_base_InStructer;
	TIM_TimeBaseStructInit(&timer_base_InStructer);
	timer_base_InStructer.TIM_ClockDivision=timerx->base->timer_div;
	timer_base_InStructer.TIM_CounterMode=timerx->base->timer_count_mode;
	timer_base_InStructer.TIM_Period=timerx->base->timer_period;
	timer_base_InStructer.TIM_Prescaler=timerx->base->timer_presc;
	timer_base_InStructer.TIM_RepetitionCounter=timerx->base->rcr_value;
	TIM_TimeBaseInit(timerx->base->timx,&timer_base_InStructer);
	
	if(timerx->isInterrupt)
	{
		//是否中断
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		NVIC_InitTypeDef NVIC_InStructer;
		memset(&NVIC_InStructer,0,sizeof(NVIC_InStructer));
		NVIC_InStructer.NVIC_IRQChannel=timerx->irqn;
		NVIC_InStructer.NVIC_IRQChannelCmd=ENABLE;
		NVIC_InStructer.NVIC_IRQChannelPreemptionPriority=15;
		NVIC_InStructer.NVIC_IRQChannelSubPriority=0;
		NVIC_Init(&NVIC_InStructer);
		TIM_ITConfig(timerx->base->timx,timerx->IT,ENABLE);
	}
	
	 if(timerx->isOC)
	{
		GPIO_InitTypeDef GPIO_InStructer;
		GPIO_StructInit(&GPIO_InStructer);
		GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
		GPIO_InStructer.GPIO_Pin=timerx->timer_output_Compare->GPIO_PIN;
		GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InStructer.GPIO_Speed=GPIO_Medium_Speed;
		GPIO_Init(timerx->timer_output_Compare->GPIO_PORT,&GPIO_InStructer);                                                                                             
		TIM_OCInitTypeDef timer_oc_InStructer;
		TIM_OCStructInit(&timer_oc_InStructer);
		timer_oc_InStructer.TIM_OCIdleState=timerx->timer_output_Compare->ocidlestate;
		timer_oc_InStructer.TIM_OCMode=timerx->timer_output_Compare->out_put_mode;
		timer_oc_InStructer.TIM_OCNIdleState=timerx->timer_output_Compare->ocnidlestate;
		timer_oc_InStructer.TIM_OCNPolarity=timerx->timer_output_Compare->ocnpolarity;
		timer_oc_InStructer.TIM_OCPolarity=timerx->timer_output_Compare->ocpolarity;
		timer_oc_InStructer.TIM_OutputNState=timerx->timer_output_Compare->out_put_nstate;
		timer_oc_InStructer.TIM_OutputState=timerx->timer_output_Compare->out_put_state;
		timer_oc_InStructer.TIM_Pulse=0;
if(timerx->base->timx == TIM1)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM1);
else if(timerx->base->timx == TIM2)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM2);
else if(timerx->base->timx == TIM3)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM3);
else if(timerx->base->timx == TIM4)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM4);
else if(timerx->base->timx == TIM5)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM5);
else if(timerx->base->timx == TIM8)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM8);
else if(timerx->base->timx == TIM9)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM9);
else if(timerx->base->timx == TIM10)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM10);
else if(timerx->base->timx == TIM11)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM11);
else if(timerx->base->timx == TIM12)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM12);
else if(timerx->base->timx == TIM13)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM13);
else if(timerx->base->timx == TIM14)
    GPIO_PinAFConfig(timerx->timer_output_Compare->GPIO_PORT,
                     timerx->timer_output_Compare->GPIO_pin_source,
                     GPIO_AF_TIM14);
		if(timerx->timer_output_Compare->channal==1)
		{
			TIM_OC1Init(timerx->base->timx,&timer_oc_InStructer);
			if(timerx->timer_output_Compare->isfast)
			{
				TIM_OC1FastConfig(timerx->base->timx,TIM_OCFast_Enable);
			}
		}
		else if(timerx->timer_output_Compare->channal==2)
		{
			TIM_OC2Init(timerx->base->timx,&timer_oc_InStructer);
			if(timerx->timer_output_Compare->isfast)
			{
				TIM_OC2FastConfig(timerx->base->timx,TIM_OCFast_Enable);
			}
		}
		else if(timerx->timer_output_Compare->channal==3)
		{
			TIM_OC3Init(timerx->base->timx,&timer_oc_InStructer);
			if(timerx->timer_output_Compare->isfast)
			{
				TIM_OC3FastConfig(timerx->base->timx,TIM_OCFast_Enable);
			}
		}
		else if(timerx->timer_output_Compare->channal==4)
		{
			TIM_OC4Init(timerx->base->timx,&timer_oc_InStructer);
			if(timerx->timer_output_Compare->isfast)
			{
				TIM_OC4FastConfig(timerx->base->timx,TIM_OCFast_Enable);
			}
		}
	}
	 
	if (timerx->isIC)
	{
		GPIO_InitTypeDef GPIO_InStructer;
		GPIO_StructInit(&GPIO_InStructer);
		GPIO_InStructer.GPIO_Mode=GPIO_Mode_AF;
		GPIO_InStructer.GPIO_Pin=timerx->intput_Caputer->GPIO_PIN1;
		GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InStructer.GPIO_Speed=GPIO_Medium_Speed;
		GPIO_Init(timerx->intput_Caputer->GPIO_PORT1,&GPIO_InStructer);
		GPIO_InStructer.GPIO_Pin=timerx->intput_Caputer->GPIO_PIN2;
		GPIO_Init(timerx->intput_Caputer->GPIO_PORT2,&GPIO_InStructer);
if(timerx->base->timx==TIM1)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM1);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM1);
}
else if(timerx->base->timx==TIM2)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM2);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM2);
}
else if(timerx->base->timx==TIM3)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM3);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM3);
}
else if(timerx->base->timx==TIM4)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM4);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM4);
}
else if(timerx->base->timx==TIM5)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM5);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM5);
}
else if(timerx->base->timx==TIM8)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM8);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM8);
}
else if(timerx->base->timx==TIM9)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM9);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM9);
}
else if(timerx->base->timx==TIM10)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM10);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM10);
}
else if(timerx->base->timx==TIM11)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM11);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM11);
}
else if(timerx->base->timx==TIM12)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM12);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM12);
}
else if(timerx->base->timx==TIM13)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM13);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM13);
}
else if(timerx->base->timx==TIM14)
{
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT1, timerx->intput_Caputer->GPIO_pin_source1, GPIO_AF_TIM14);
    GPIO_PinAFConfig(timerx->intput_Caputer->GPIO_PORT2, timerx->intput_Caputer->GPIO_pin_source2, GPIO_AF_TIM14);
}
		TIM_ICInitTypeDef timer_icInStructer;
		TIM_ICStructInit(&timer_icInStructer);
		timer_icInStructer.TIM_Channel=timerx->intput_Caputer->tim_channel1;
		timer_icInStructer.TIM_ICFilter=timerx->intput_Caputer->filter1;
		timer_icInStructer.TIM_ICPolarity=timerx->intput_Caputer->tim_polarity1;
		timer_icInStructer.TIM_ICPrescaler=timerx->intput_Caputer->prescaler1;
		timer_icInStructer.TIM_ICSelection=timerx->intput_Caputer->tim_selection1;
		TIM_ICInit(timerx->base->timx,&timer_icInStructer);
		timer_icInStructer.TIM_Channel=timerx->intput_Caputer->tim_channel2;
		timer_icInStructer.TIM_ICFilter=timerx->intput_Caputer->filter2;
		timer_icInStructer.TIM_ICPolarity=timerx->intput_Caputer->tim_polarity2;
		timer_icInStructer.TIM_ICPrescaler=timerx->intput_Caputer->prescaler2;
		timer_icInStructer.TIM_ICSelection=timerx->intput_Caputer->tim_selection2;
		TIM_ICInit(timerx->base->timx,&timer_icInStructer);
		if(timerx->intput_Caputer->slave_mode)
		{
			//设置从模式
			TIM_SelectSlaveMode(timerx->base->timx,timerx->intput_Caputer->slave_mode);
			TIM_SelectInputTrigger(timerx->base->timx,timerx->intput_Caputer->trigger_source);
		}
	}
	TIM_Cmd(timerx->base->timx,ENABLE);
}

float get_duty(timer_handler timerx)
{
		if(!timerx->isIC)
		{
			return 0;
		}
		uint32_t ic1=TIM_GetCapture1(timerx->base->timx);
		uint32_t ic2=TIM_GetCapture2(timerx->base->timx);
		if(ic1)
		{
			return (float)ic2*100/ic1;
		}
		else 
		{
			return 0;
		}
}

float get_frequence(timer_handler timerx)
{
		if(!timerx->isIC)
		{
			return 0;
		}
		RCC_ClocksTypeDef RCC_Clocks;
		RCC_GetClocksFreq(&RCC_Clocks);
		//定时器会将总线的频率倍频
		uint32_t tim_apb_clk_hz;
		if(timerx->base->timx==TIM1||timerx->base->timx==TIM8
			||timerx->base->timx==TIM9||timerx->base->timx==TIM10||timerx->base->timx==TIM11)
		{
			tim_apb_clk_hz = RCC_Clocks.PCLK2_Frequency * 2ul;
		}
		else{
		tim_apb_clk_hz = RCC_Clocks.PCLK1_Frequency * 2ul;
		}
		uint32_t ic1=TIM_GetCapture1(timerx->base->timx);
		if(ic1)
		{
			return (float)tim_apb_clk_hz/(timerx->base->timer_presc+1)/(timerx->intput_Caputer->prescaler1+1)/ic1;
		}
		else 
		{
			return 0;
		}
}

void set_compare(timer_handler timerx,uint32_t value)
{
		TIM_SetCompare1(timerx->base->timx,value);
}

void tim_callback_register(timer_handler timerx,timer_callback_t func)
{
	if(timerx&&func)
	{
		timerx->base->callback=func;
	}
}

void tim_ic_callback_register(timer_handler timerx,timer_ic_callback_t func)
{
	if(timerx&&func)
	{
		timerx->intput_Caputer->callback=func;
	}
}

//void tim_callback_register_tick(tim_callback_func func)
//{
//	if(func)
//	{
//		func_callback=func;
//	}
//}


//volatile uint8_t flag=0;
uint64_t tick_now_tick(void)
{
		uint64_t last_tick=tick;
		uint64_t tick_now_tick=tick;
		do{
		last_tick=tick;
		tick_now_tick=tick+TIM_GetCounter(TIM6);
	}while(tick!=last_tick);
	return tick_now_tick;
	
//		return TIM_GetCounter(TIM6);
}

uint64_t tick_get_ms(void)
{
	return tick_now_tick()/TICKS_PER_MS;
}

uint64_t tick_get_us(void)
{
	return tick_now_tick()/TICKS_PER_US;
}

void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)==SET)
	{
		//printf("enter\r\n");
		tick+=TICKS_PER_MS;
//		if(func_callback)
//		{
//			func_callback();
//		}
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
	}
}
//void TIM4_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM4,TIM_IT_CC1)==SET)
//	{
//		timer_handler_3->intput_Caputer->callback(timer_handler_3,USART_desc_1);
//		TIM_ClearITPendingBit(TIM4,TIM_IT_CC1);
//	}
//}
