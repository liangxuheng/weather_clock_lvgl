#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"
#include "led.h"
#define TICK_PER_MS (SystemCoreClock/1000)
#define TICK_PER_US (SystemCoreClock/1000/1000)
volatile uint64_t tick_count=0;
extern led_desc_t led3;
typedef void(*systick_callback_t)(void);
static systick_callback_t systick_callback_func=NULL;
void systick_my_init()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk;
	SysTick->VAL=0;
	SysTick->LOAD=(TICK_PER_MS)-1;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
	//内核中断中断号(irqn)是小于零的，用结构体的方式的话结构体
	//的这个成员是uint8_t类型的，所以不行
	NVIC_SetPriority(SysTick_IRQn,0);
}

uint64_t tick_now(void)
{
	uint64_t last=0;
	uint64_t now=0;
	do 
	{
		last=tick_count;
		now=tick_count+SysTick->LOAD-SysTick->VAL;
	}while(last!=tick_count);
	return now;
}

uint64_t get_ms(void)
{
	return tick_now()/TICK_PER_MS;
}

uint64_t get_us(void)
{
	return tick_now()/TICK_PER_US;
}

//void SysTick_Handler(void)
//{
//	tick_count+=TICK_PER_MS;
//	if(systick_callback_func)
//	{
//		systick_callback_func();
//	}
//	//这里可以不用清(清了也没有问题)
//	NVIC_ClearPendingIRQ(SysTick_IRQn);
//}

void Systick_callback_register(systick_callback_t func)
{
	if(func)
	{
		systick_callback_func=func;
	}
}
