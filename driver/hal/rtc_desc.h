#ifndef __RTC_DESC_H_
#define __RTC_DESC_H_
#include <stdint.h>
struct rtc_struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
};
#endif
