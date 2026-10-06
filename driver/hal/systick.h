#ifndef __SYSTICK_MY_H__
#define __SYSTICK_MY_H__
#include <stdint.h>
#define TICK_PER_MS (SystemCoreClock/1000)
#define TICK_PER_US (SystemCoreClock/1000/1000)
typedef void(*systick_callback_t)(void);
void Systick_callback_register(systick_callback_t func);
void systick_my_init(void);
uint64_t tick_now(void);
uint64_t get_ms(void);
uint64_t get_us(void);
#endif
