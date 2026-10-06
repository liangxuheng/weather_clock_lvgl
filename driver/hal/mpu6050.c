#include "stm32f4xx.h"
#include "mpu6050.h"
#include "i2c.h"
#include "mpu6050_desc.h"
#include <stdio.h>
//芯片的寄存器列表
#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG			0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75


#define SLAVE_ADDRESS 0xD0
static bool mpu6050_write(i2c_handle i2cx,uint8_t address_slave,uint8_t address,uint8_t *data,uint32_t length)
{
		if(i2c_write(i2cx,address_slave,address,data,length))
		{
			return true;
		}
		return false;
}

static bool mpu6050_read(i2c_handle i2cx,uint8_t address_slave,uint8_t address,uint8_t *data,uint32_t length)
{
		if(i2c_read(i2cx,address_slave,address,data,length))
		{
			return true;
		}
		return false;
}

void MPU0650_init(i2c_handle i2cx)
{
		m_i2c_init(i2cx);
		uint8_t data=0;
		data=0x01;
		mpu6050_write(i2cx,SLAVE_ADDRESS,MPU6050_PWR_MGMT_1,&data,sizeof(uint8_t));
		data=0x00;
		mpu6050_write(i2cx,SLAVE_ADDRESS,MPU6050_PWR_MGMT_2,&data,sizeof(uint8_t));
		data=0x09;
		mpu6050_write(i2cx,SLAVE_ADDRESS,MPU6050_SMPLRT_DIV,&data,sizeof(uint8_t));
		data=0x06;
		mpu6050_write(i2cx,SLAVE_ADDRESS,MPU6050_CONFIG,&data,sizeof(uint8_t));
		data=0x18;
		mpu6050_write(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_CONFIG,&data,sizeof(uint8_t));
}

bool MPU6050_ReadREG(i2c_handle i2cx,mpu6050_handler mpu6050)
{
	uint8_t high=0;
	uint8_t low=0;
	if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_XOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_XOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Accx=(high<<8)|low;
	}
	else{
		return false;
	}
	
		if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_YOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_YOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Accy=(high<<8)|low;
	}
	else{
		return false;
	}
	
			if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_ZOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_ACCEL_ZOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Accz=(high<<8)|low;
	}
	else{
		return false;
	}
	
			if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_XOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_XOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Grox=(high<<8)|low;
	}
	else{
		return false;
	}
	
				if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_YOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_YOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Groy=(high<<8)|low;
	}
	else{
		return false;
	}
	
				if(mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_ZOUT_H,&high,sizeof(uint8_t))
		&&mpu6050_read(i2cx,SLAVE_ADDRESS,MPU6050_GYRO_ZOUT_L,&low,sizeof(uint8_t)))
	{
		mpu6050->Groz=(high<<8)|low;
	}
	else{
		return false;
	}
	return true;
}


//Accx
//Accy
//Accz
//Grox
//Groy
//Groz


void MPU6050_showdata(mpu6050_handler mpu6050,uint8_t *buf,uint32_t length)
{
	if(mpu6050&&length>0&&buf)
	{snprintf((char*)buf,length,"accx:%d,accy:%d,accz:%d,grox:%d,groy:%d,groz:%d\r\n"\
		,mpu6050->Accx,mpu6050->Accy,mpu6050->Accz,mpu6050->Grox,mpu6050->Groy,mpu6050->Groz);
	}
}
