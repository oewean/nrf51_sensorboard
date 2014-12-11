#include "app_max44009.h"
#include "hal_twi.h"

#define BUF_LEN            (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = 0;

uint32_t app_max44009_lux_read(uint8_t * p_exponent, uint8_t * p_mantissa)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);

    // Select LUX (ambient light) register for reading
    data[0] = MAX44009_RA_LUX_H;
    err_code = hal_twi_write(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Read out register value
    err_code = hal_twi_read(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Mask out and shift result
    *p_exponent = (data[0] >> 4) & 0x0f;
    *p_mantissa = (data[0] << 4) & 0xf0; // TODO: Check aditional mantissa bytes. Gives same as high-byte mantissa: | (data[1] & 0x0f);

    return 0;
}


uint32_t app_max44009_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return 0;
}

