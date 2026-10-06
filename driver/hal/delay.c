#include "delay.h"
#include "stm32f4xx.h"
#include "timer.h"
void delay(uint32_t us)
{
//	//使用systick进行延时(单纯的硬件延时只能达到99.86ms)
//	//使能控制寄存器并选择内部时钟作为时钟源
//	SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_ENABLE_Msk;
//	//对当前值寄存器执行写操作无论写什么都会清零
//	SysTick->VAL=0;
//	//(SystemCoreClock/1000/1000)先转化成毫秒,
//	//为什么不将写成us*SystemCoreClock/1000/1000,因为这样可能在*时就溢出了
//	//这里减不减一影响不大，减一就更严谨一点
//	SysTick->LOAD=(SystemCoreClock/1000/1000)*us-1;
//	//等待计数完成
//	while((SysTick->CTRL&SysTick_CTRL_COUNTFLAG_Msk)==0);
//	//关闭
//	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;

		uint64_t last=tick_now_tick();
	while(tick_now_tick()-last<us*TICKS_PER_US);
}

void delay_ms(uint32_t ms)
{
	for(uint32_t i=0;i<1000;++i)
	{
		delay(ms);
	}
}

void delay_s(uint32_t s)
{
	for(uint32_t i=0;i<1000;++i)
	{
		delay_ms(s);
	}
}
