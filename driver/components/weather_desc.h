#ifndef __WEATHEE_DESC_H_
#define __WEATHEE_DESC_H_
struct weather{
	char city[32];
	char loaction[128];
	char weather[16];
	int weather_code;
	float temperature;
};
#endif
