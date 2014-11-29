#ifndef APP_MAX44009_H__
#define APP_MAX44009_H__

#include <stdint.h>
#include <stdbool.h>

uint32_t app_max44009_lux_read(uint8_t * exponent, uint8_t * mantissa);
uint32_t app_max44009_init(uint8_t i2c_addr);

#endif // APP_MAX44009_H__
