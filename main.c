#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "mpu9250.h"
#include "lcd_1_8.h"

#define AVG_AMT 20

static mpu9250_sensor_reading_t accel[2] = {
  { 0.0f, 0.0f, 0.0f },
  { 0.0f, 0.0f, 0.0f }
};
static uint8_t read_index = 0;

void core1_entry() {
  mpu9250_sensor_reading_t accel_readings[AVG_AMT];
  uint8_t accel_readings_index = 0;

  memset(accel_readings, 0, sizeof(mpu9250_sensor_reading_t) * AVG_AMT);

  printf("Initialise MPU9250\n");
  if (!mpu9250_init()) {
    printf("MPU9250 initialisation failed\n");
    return;
  }
  printf("MPU9250 initialised\n");

  while (1) {
    uint8_t write_index = (read_index + 1) & 1;
    mpu9250_read_all(&accel_readings[accel_readings_index++], NULL, NULL);
    accel_readings_index = accel_readings_index % AVG_AMT;
    accel[write_index].x = 0.0f;
    accel[write_index].y = 0.0f;
    accel[write_index].z = 0.0f;
    for (uint8_t i = 0; i < AVG_AMT; i++) {
      accel[write_index].x += accel_readings[i].x;
      accel[write_index].y += accel_readings[i].y;
      accel[write_index].z += accel_readings[i].z;
    }
    read_index = write_index;
    printf("%f %f %f\r", accel[write_index].x, accel[write_index].y, accel[write_index].z);
  }
}

int main(void) {
  lcd_1_8_t lcd_1_8_ctx;
  lcd_1_8_coord_t start = { .x = 80, .y = 60 };
  lcd_1_8_coord_t end = { .x = 80, .y = 60 };

	stdio_init_all();

  sleep_ms(2000);

  printf("Initialise LCD\n");
  if (!lcd_1_8_init(&lcd_1_8_ctx)) {
    printf("LCD initialisation failed\n");
    return -1;
  }
  printf("LCD initialised\n");

  multicore_launch_core1(core1_entry);

  while(1){
    lcd_1_8_line(&start, &end, 0x0000, 1, 0);
    lcd_1_8_circle(&end, 5, 0x0000, 1, 1);
    end.x = (float) 80 - (float) 5 * accel[read_index].y / (float) AVG_AMT;
    end.y = (float) 60 + (float) 5 * accel[read_index].x / (float) AVG_AMT;
    lcd_1_8_line(&start, &end, 0xffff, 1, 0);
    lcd_1_8_circle(&end, 5, 0xffff, 1, 1);
    lcd_1_8_update(&lcd_1_8_ctx);
    sleep_ms(10);
  }

  return 0;
}
