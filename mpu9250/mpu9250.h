#ifndef _MPU_9250_H_
#define _MPU_9250_H_

typedef struct {
  float x;
  float y;
  float z;
} mpu9250_sensor_reading_t;

bool mpu9250_init();
void mpu9250_read_all(mpu9250_sensor_reading_t *accel, mpu9250_sensor_reading_t *gyro, mpu9250_sensor_reading_t *magn);

#endif // _MPU_9250_H_
