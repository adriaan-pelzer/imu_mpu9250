#include <stdio.h>
#include <math.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "mpu9250_addr.h"
#include "mpu9250.h"

#define I2C_PORT i2c1

static char i2c_read_one(char address, char reg) {
  char rc;
  i2c_write_blocking(i2c1, address, &reg, 1, true);
  i2c_read_blocking(i2c1, address, &rc, 1, false);
  return rc;
}

static void i2c_read_many(char address, char reg, char *values, unsigned int length) {
  i2c_write_blocking(i2c1, address, &reg, 1, true);
  i2c_read_blocking(i2c1, address, values, length, false);
}

static void i2c_write_one(char address, char reg, char value) {
  char buf[] = { reg, value };
  i2c_write_blocking(i2c1, address, buf, 2, false);
}

static void get_sensor_readings_6dof(int16_t *readings) {
  char buf[14];
  i2c_read_many(GYRO_ADDRESS, ACCEL_XOUT_H, buf, 14);
  for (uint8_t i = 0; i < 3; i++) {
    readings[i+0] = (((int16_t) buf[2*i+0]) << 8) | buf[2*i+1];
    readings[i+3] = (((int16_t) buf[2*i+8]) << 8) | buf[2*i+9];
  }
}

static void get_sensor_readings_9dof(int16_t *readings) {
  char buf[6];
  get_sensor_readings_6dof(readings);
  i2c_write_one(GYRO_ADDRESS, INT_PIN_CFG, 0x02);
  i2c_write_one(MAG_ADDRESS, MAG_CNTL1, 0x01);
  sleep_ms(10);
  i2c_read_many(MAG_ADDRESS, MAG_XOUT_L, buf, 6);
  for (uint8_t i = 0; i < 3; i++) {
    readings[i+6] = (((int16_t) buf[2*i+1]) << 8) | buf[2*i+0];
  }
}

bool mpu9250_init() {
  i2c_init(I2C_PORT, 400*1000);
  gpio_set_function(6, GPIO_FUNC_I2C);
  gpio_set_function(7, GPIO_FUNC_I2C);
  gpio_pull_up(6);
  gpio_pull_up(7);
  i2c_write_one(GYRO_ADDRESS, PWR_MGMT, 0x01);
  i2c_write_one(GYRO_ADDRESS, GYRO_CONFIG, 0x00);
  i2c_write_one(GYRO_ADDRESS, ACCEL_CONFIG1, 0x00);
  i2c_write_one(GYRO_ADDRESS, PWR_MGMT, 0x01);
  if (i2c_read_one(GYRO_ADDRESS, GYRO_WHOAMI) != GYRO_ID) {
    printf("initialisation failed\n");
    return false;
  }
  return true;
}

void mpu9250_calibrate_gyro(mpu9250_sensor_reading_t *gyro_offset) {
  for (uint8_t i = 0; i < 3; i++) {
    ((float *) gyro_offset)[i] = 0.0f;
  }
  for (uint8_t i = 5; i > 0; i--) {
    printf("keep sensor still %u ...\r", i);
    sleep_ms(1000);
  }
  printf("\n");
  for (uint16_t i = 0; i < 1000; i++) {
    int16_t readings[6];
    get_sensor_readings_6dof(readings);
    for (uint8_t j = 0; j < 3; j++) {
      ((float *) gyro_offset)[j] += (float) readings[j+3];
    }
  }
  for (uint8_t i = 0; i < 3; i++) {
    ((float *) gyro_offset)[i] /= 1000.0f;
  }
}

void mpu9250_calibrate_magn(mpu9250_sensor_reading_t *magn_readings) {
  for (uint8_t i = 5; i > 0; i--) {
    printf("turn sensor slowly in 3D %u ...\r", i);
    sleep_ms(1000);
  }
  printf("\n");
  for (uint16_t i = 0; i < 1000; i++) {
    int16_t readings[9];
    get_sensor_readings_9dof(readings);
    for (uint8_t j = 0; j < 3; j++) {
      ((float *) &magn_readings[i])[j] = readings[j+6];
    }
  }
  printf("done\n");
}

void mpu9250_read_all(mpu9250_sensor_reading_t *accel, mpu9250_sensor_reading_t *gyro, mpu9250_sensor_reading_t *magn) {
  int16_t raw_readings[9];
  float mag_intermediate[3];
  float accel_scale = 9.8f / 16500.0f;
  float gyro_scale = 250.0f / 32768.0f * (M_PI/180.0f);
  float gyro_offset[3] = { -190.399994, -66.570000, -179.013000 };
  float mag_bias[3] = { 28.406431, 57.577494, -96.531941 };
  float mag_correction_matrix[3][3] = {
    { 1.038150, 0.014456, -0.017427 },
    { 0.014456, 1.000001, -0.013259 },
    { -0.017427, -0.013259, 0.976548 }
  };
  /*float mag_bias[3] = { 1.0, 1.0, 1.0 };
  float mag_correction_matrix[3][3] = {
    { 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0 }
  };*/

  get_sensor_readings_9dof(raw_readings);

  for (uint8_t i = 0; i < 3; i++) {
    if (accel) {
      ((float *) accel)[i] = ((float) raw_readings[i+0] * accel_scale);
    }
    if (gyro) {
      ((float *) gyro)[i] = ((float) raw_readings[i+3] - gyro_offset[i]) * gyro_scale;
    }
    if (magn) {
      mag_intermediate[i] = (float) raw_readings[i+6] - mag_bias[i];
    }
  }

  if (magn) {
    //float mag_total = 0.0f;
    for (uint8_t i = 0; i < 3; i++) {
      ((float *) magn)[i] = 0.0f;
      for (uint8_t j = 0; j < 3; j++) {
        ((float *) magn)[i] += mag_intermediate[j] * mag_correction_matrix[i][j];
      }
      //mag_total += ((float *) magn)[i] * ((float *) magn)[i];
    }
    /*mag_total /= 3.0f;
    mag_total = sqrtf(mag_total);
    for (uint8_t i = 0; i < 3; i++) {
      ((float *) magn)[i] /= mag_total;
    }*/
  }
}
