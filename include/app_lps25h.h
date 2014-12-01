#ifndef APP_LPS25H_H__
#define APP_LPS25H_H__

#include <stdint.h>
//#include <stdbool.h>

uint32_t app_lps25h_press_read(int32_t * p_meas);
uint32_t app_lps25h_temp_read(int16_t * p_meas);

uint32_t app_lps25h_init(uint8_t i2c_addr);

#endif // APP_LPS25H_H__
