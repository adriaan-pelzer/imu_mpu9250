aux_source_directory(. DIR_mpu9250_SRCS)

add_library(mpu9250 ${DIR_mpu9250_SRCS})
target_link_libraries(mpu9250 PUBLIC hardware_i2c pico_stdlib)
