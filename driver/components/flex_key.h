#ifndef __FLEX_KEY_H_
#define __FLEX_KEY_H_
#include <stdbool.h>
#include <stdint.h>
#define FLEX_KEY_SCAN_HZ 50

#define FlEX_KEY_MS_TO_SCANTICK(ms)\
((ms)/(1000/(FLEX_KEY_SCAN_HZ)))
typedef enum
{
    FLEX_BTN_PRESS_DOWN = 0,
    FLEX_BTN_PRESS_CLICK,
    FLEX_BTN_PRESS_DOUBLE_CLICK,
    FLEX_BTN_PRESS_REPEAT_CLICK,
    FLEX_BTN_PRESS_SHORT_START,
    FLEX_BTN_PRESS_SHORT_UP,
    FLEX_BTN_PRESS_LONG_START,
    FLEX_BTN_PRESS_LONG_UP,
    FLEX_BTN_PRESS_LONG_HOLD,
    FLEX_BTN_PRESS_LONG_HOLD_UP,
    FLEX_BTN_PRESS_MAX,
    FLEX_BTN_PRESS_NONE,
} flex_button_event_t;
struct flex_button;
typedef struct flex_button* flex_button_handler;
typedef void(*flex_button_callback)(void*);
bool flex_button_init(flex_button_handler button,uint8_t length
	,flex_button_callback cb,uint16_t ms);
flex_button_event_t flex_button_event_read(flex_button_handler button);
#endif
