#include <stdbool.h>
#include "led_desc.h"
#include "led.h"
#include <string.h>
void led_init(led_desc_t ledx)
{
	GPIO_InitTypeDef GPIO_InStructer;
	memset(&GPIO_InStructer,0,sizeof(GPIO_InStructer));
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InStructer.GPIO_OType=GPIO_OType_PP;
	GPIO_InStructer.GPIO_Pin=ledx->GPIO_PIN;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InStructer.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(ledx->GPIO_PORT,&GPIO_InStructer);
}

static void led_set(led_desc_t ledx,bool ison)
{
		//内部函数
	GPIO_WriteBit(ledx->GPIO_PORT,\
	ledx->GPIO_PIN,ison?ledx->on:ledx->off);
}

void led_on(led_desc_t ledx){
	//公共函数,要先检查参数是否合法
	if(ledx==NULL)
	{
		return;
	}
	led_set(ledx,true);
}

void led_off(led_desc_t ledx)
{
	if(ledx==NULL)
	{
		return;
	}
	led_set(ledx,false);
}
