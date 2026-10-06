#ifndef __MPU6050_DESC_H_
#define __MPU6050_DESC_H_
#include <stdint.h>
struct REG;
struct REG
{
	int16_t Accx;
	int16_t Accy;
	int16_t Accz;
	int16_t Grox;
	int16_t Groy;
	int16_t Groz;
};

#endif
