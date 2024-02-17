cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(pico-10dof-imu_project)

pico_sdk_init()

add_subdirectory(mpu9250)
add_subdirectory(lcd_1_8)
include_directories(./mpu9250)
include_directories(./lcd_1_8)

add_executable(imu main.c)

pico_enable_stdio_usb(imu 1)
pico_enable_stdio_uart(imu 1)

pico_add_extra_outputs(imu)

target_link_libraries(imu pico_stdlib hardware_i2c pico_multicore mpu9250 lcd_1_8)
