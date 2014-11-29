#include "app_max44009.h"
#include "twi_master.h"

#define BUF_LEN             (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = 0;

uint32_t app_max44009_lux_read(uint8_t * p_exponent, uint8_t * p_mantissa)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // Select LUX (ambient light) register for reading
    data_buffer[0] = 3; // Lux High Byte register addr
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 1, TWI_DONT_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Read out register value
    success = twi_master_transfer((m_i2c_addr << 1) | TWI_READ_BIT, data_buffer, 2, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Mask out and shift result
    *p_exponent = (data_buffer[0] >> 4) & 0x0f;
    *p_mantissa = (data_buffer[0] << 4) & 0xf0; // TODO: Check aditional mantissa bytes. Gives same as high-byte mantissa: | (data_buffer[1] & 0x0f);

    return 0;
}


uint32_t app_max44009_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return 0;
}

