#ifndef __RTC_M_H_
#define __RTC_M_H_
struct rtc_struct;
typedef struct rtc_struct* rtc_handler;
void rtc_init(void);
void rtc_get_time(rtc_handler rtcx);
void rtc_set_time(rtc_handler rtcx);
#endif
