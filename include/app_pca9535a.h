#ifndef APP_PCA9535A_H__
#define APP_PCA9535A_H__

#include <stdint.h>
#include <stdbool.h>

#define BUTTON0_MASK (1UL << 0)
#define BUTTON1_MASK (1UL << 1)

uint32_t app_pca9535a_init(uint8_t i2c_addr);
uint32_t app_pca9535a_led0(bool on);
uint32_t app_pca9535a_port_config_get(uint8_t * port0, uint8_t * port1);
uint32_t app_pca9535a_input_state_get(uint8_t * port0, uint8_t * port1);
uint32_t app_pca9535a_output_state_get(uint8_t * port0, uint8_t * port1);

#endif // APP_PCA9535A_H__
