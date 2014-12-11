#include "app_mcp9808.h"
#include "hal_twi.h"

#define BUF_LEN (32)    // TODO: Find actual size
static uint8_t m_i2c_addr;

uint32_t app_mcp9808_temp_read(int16_t * p_meas)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    // Select T_A (ambient temperature) register for reading
    data[0] = MCP9808_RA_TEMP;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);
    err_code = hal_twi_write(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Read out register value
    err_code = hal_twi_read(2, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Mask out flags and clean up sign bit
    *p_meas = data[0] << 8 | data[1];
    *p_meas &= 0x1FFF;                // Clear flag bits
    if ((*p_meas & 0x1000) == 0x1000) //TA < 0°C
    {
        *p_meas |= 0xF000;            // Pad sign bit
    }

    return 0;
}


uint32_t app_mcp9808_shutdown(bool turn_off)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    // Write shutdown bit in config register
    data[0] = MCP9808_RA_CONFIG;
    data[1] = turn_off ? 1 : 0; // Set shutdown bit (and set T_HYST to default values)

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_TX_BUF_END);
    err_code = hal_twi_write(2, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    return 0;
}


uint32_t app_mcp9808_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return 0;
}

