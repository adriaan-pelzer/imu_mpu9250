#ifndef __MPU9250_ADDR_H
#define __MPU9250_ADDR_H

#define GYRO_ADDRESS  0x68
#define GYRO_WHOAMI   0x75
#define GYRO_ID       0x71
#define PWR_MGMT      0x6B
#define CONFIG        0x1A
#define GYRO_CONFIG   0x1B
#define ACCEL_CONFIG1 0x1C
#define ACCEL_CONFIG2 0x1D
#define USER_CTRL     0x6A
#define INT_PIN_CFG   0x37

#define MAG_ADDRESS   0x0C
#define MAG_WHOAMI    0x00
#define MAG_ID        0x48
#define MAG_CNTL1     0x0A
#define MAG_CNTL2     0x0B
#define MAG_ST1       0x02

#define ACCEL_XOUT_H  0x3B
#define GYRO_XOUT_H   0x43
#define MAG_ASAX      0x10
#define MAG_XOUT_L    0x03

#endif // __MPU9250_ADDR_H
