#include "flex_desc.h"
#include "flex_key.h"

uint8_t usr_key_read(void *p)
{
	flex_button_handler flex_button_x=(flex_button_handler)p;
	return GPIO_ReadInputDataBit(flex_button_x->gpio->GPIO_PORT
	,flex_button_x->gpio->GPIO_PIN);
}

void gpio_init(flex_button_handler flex_button_x)
{
	GPIO_InitTypeDef GPIO_InStructer;
	GPIO_StructInit(&GPIO_InStructer);
	GPIO_InStructer.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InStructer.GPIO_Pin=flex_button_x->gpio->GPIO_PIN;
	GPIO_InStructer.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InStructer.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(flex_button_x->gpio->GPIO_PORT,&GPIO_InStructer);
}
