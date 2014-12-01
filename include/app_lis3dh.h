#ifndef APP_LIS3DH_H__
#define APP_LIS3DH_H__

#include <stdint.h>
//#include <stdbool.h>

//uint32_t app_mcp9808_temp_read(int16_t * p_meas);

uint32_t app_lis3dh_init(uint8_t i2c_addr);

#endif // APP_LIS3DH_H__
