#ifndef __MPU6050_H_
#define __MPU6050_H_
#include <stdint.h>
#include <stdbool.h>
struct my_i2c;
typedef struct my_i2c* i2c_handle;
struct REG;
typedef struct REG* mpu6050_handler;
void MPU0650_init(i2c_handle i2cx);
bool MPU6050_ReadREG(i2c_handle i2cx,mpu6050_handler mpu6050);
void MPU6050_showdata(mpu6050_handler mpu6050,uint8_t *buf,uint32_t length);
#endif
