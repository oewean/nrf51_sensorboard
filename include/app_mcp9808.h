#ifndef APP_MCP9808_H__
#define APP_MCP9808_H__

#include <stdint.h>
#include <stdbool.h>

#define MCP9808_RA_CONFIG 0x01
#define MCP9808_RA_TEMP   0x05

/**@brief Application MCP9808 Temperature Measurment Read.
 *
 * @param[out]   meas  MCP9808 temperature measurement (unit 0.125°C pr default).
 *
 * @retval  0          Successfully read temperature.
 */
uint32_t app_mcp9808_temp_read(int16_t * p_meas);

uint32_t app_mcp9808_shutdown(bool turn_off);

/**@brief Initialize the MCP9808 temperature measurement module.
 *
 * @param[in]    i2c_addr  I2C addr of the MCP9808 device.
 *
 */
uint32_t app_mcp9808_init(uint8_t i2c_addr);

#endif // APP_MCP9808_H__
