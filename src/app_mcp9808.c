#include "app_mcp9808.h"
#include <stdlib.h>
#include <string.h>
#include "twi_master.h"

#define BUF_LEN             (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = NULL;

uint32_t app_mcp9808_temp_read(int16_t * p_meas)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // Select T_A (ambient temperature) register for reading
    data_buffer[0] = 5; // T_A register addr
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

    // Mask out flags and clean up sign bit
    *p_meas = data_buffer[0] << 8 | data_buffer[1];
    *p_meas &= 0x1FFF; // Clear flag bits
    if ((*p_meas & 0x1000) == 0x1000)
    { //TA < 0°C
        *p_meas |= 0xF000; // Pad sign bit
    }

    return 0;
}


uint32_t app_mcp9808_shutdown(bool turn_off)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // Write shutdown bit in config register
    data_buffer[0] = 1;
    data_buffer[1] = turn_off ? 1 : 0;
    data_buffer[2] = 0;
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 3, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    return 0;
}


uint32_t app_mcp9808_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return 0;
}

