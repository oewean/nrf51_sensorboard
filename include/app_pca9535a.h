#ifndef APP_PCA9535A_H__
#define APP_PCA9535A_H__

#include <stdint.h>
#include <stdbool.h>

#define PCA9535A_RA_INPUT_0 0x00
#define PCA9535A_RA_INPUT_1 0x01
#define PCA9535A_RA_OUTPUT_0 0x02
#define PCA9535A_RA_OUTPUT_1 0x03
#define PCA9535A_RA_CONFIG_0 0x06
#define PCA9535A_RA_CONFIG_1 0x07

#define PCA9535A_BUTTON0_MASK (1UL << 0)
#define PCA9535A_BUTTON1_MASK (1UL << 1)

uint32_t app_pca9535a_init(uint8_t i2c_addr);
uint32_t app_pca9535a_led0(bool on);
uint32_t app_pca9535a_port_config_get(uint8_t * port0, uint8_t * port1);
uint32_t app_pca9535a_input_state_get(uint8_t * port0, uint8_t * port1);
uint32_t app_pca9535a_output_state_get(uint8_t * port0, uint8_t * port1);

#endif // APP_PCA9535A_H__
