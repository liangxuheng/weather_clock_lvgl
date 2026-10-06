#ifndef __WEATHER_H_
#define __WEATHER_H_
#include <stdbool.h>
struct weather;
typedef struct weather* weather_handler;
struct esp32_c3;
typedef struct esp32_c3* esp32c3_handler;
bool get_weather(esp32c3_handler esp32x,weather_handler weather);
const char* get_weather_city(const weather_handler w);
const char* get_weather_location(const weather_handler w);
const char* get_weather_text(const weather_handler w);
int get_weather_code(const weather_handler w);
float get_weather_temperature(const weather_handler w);
#endif
