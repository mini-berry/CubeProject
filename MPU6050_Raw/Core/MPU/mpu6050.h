#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"

#define SMPLRT_DIV   0x19 // 采样率分频，典型值：0x07(125Hz) */
#define CONFIG       0x1A // 低通滤波频率，典型值：0x06(5Hz) */
#define GYRO_CONFIG  0x1B // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s) */
#define ACCEL_CONFIG 0x1C // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz) */

#define ACCEL_XOUT_H 0x3B // 存储最近的X轴、Y轴、Z轴加速度感应器的测量值 */
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

#define TEMP_OUT_H   0x41 // 存储的最近温度传感器的测量值 */
#define TEMP_OUT_L   0x42

#define GYRO_XOUT_H  0x43 // 存储最近的X轴、Y轴、Z轴陀螺仪感应器的测量值 */
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48

#define PWR_MGMT_1   0x6B // 电源管理，典型值：0x00(正常启用) */
#define WHO_AM_I     0x75 // IIC地址寄存器(默认数值0x68，只读) */
#define MPU6050_ADDR 0xD0 // MPU6050手册上的地址，这里也可以使用serch函数去搜索

typedef struct {
    // 角速度
    float Accel_X;
    float Accel_Y;
    float Accel_Z;
    // 角度
    float Gyro_X;
    float Gyro_Y;
    float Gyro_Z;
    // 温度
    float Temp;
} MPU6050DATATYPE;

extern MPU6050DATATYPE RawData;
extern I2C_HandleTypeDef hi2c1;

int16_t Sensor_I2C2_Serch();
int8_t MPU6050_Init(int16_t Addr);

void MPU6050_Read_Accel();
void MPU6050_Read_Gyro();
void MPU6050_Read_Temp();

#endif
