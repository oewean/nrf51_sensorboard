#ifndef APP_MPU9150_H__
#define APP_MPU9150_H__

#include <stdint.h>

//uint32_t app_mcp9808_temp_read(int16_t * p_meas);

uint32_t app_mpu9150_init(uint8_t i2c_addr);

#endif // APP_MPU9150_H__
