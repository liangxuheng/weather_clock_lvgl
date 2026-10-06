#ifndef __TIMER_H_
#define __TIMER_H_
#include <stdint.h>
typedef void(*tim_callback_func)(void);
//typedef void (*timer_callback_t)(USART_MY_handle);
//typedef struct timer_struct* timer_handler;
struct timer_struct;
typedef struct timer_struct* timer_handler;
struct USART_desc;
typedef  struct USART_desc* USART_MY_handle;
typedef void (*timer_ic_callback_t)(timer_handler,USART_MY_handle);
typedef void (*timer_callback_t)(USART_MY_handle);
void tim_callback_register(timer_handler timerx,timer_callback_t func);
void m_time_init(timer_handler timerx);
float get_frequence(timer_handler timerx);
float get_duty(timer_handler timerx);
void set_compare(timer_handler timerx,uint32_t value);
void tim_ic_callback_register(timer_handler timerx,timer_ic_callback_t func);
uint64_t tick_now_tick(void);
uint64_t tick_get_ms(void);
uint64_t tick_get_us(void);
void tim_callback_register_tick(tim_callback_func func);
#define TICKS_PER_MS (1000)
#define TICKS_PER_US (1)
#endif
