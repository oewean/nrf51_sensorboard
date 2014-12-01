#include "app_pca9535a.h"
#include <stdlib.h>
//#include <string.h>
#include "twi_master.h"

#define BUF_LEN             (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = NULL;

uint32_t app_pca9535a_led0(bool on)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // TODO: read first
    // Write led bit in config register
    data_buffer[0] = 2;
    data_buffer[1] = 0xff ^ ( on ? (1 << 2) : 0 );
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 2, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    return 0;
}

uint32_t m_read_register_pairs(uint8_t reg_addr, uint8_t * port0, uint8_t * port1)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // Select register pair for reading
    data_buffer[0] = reg_addr;
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 1, TWI_DONT_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Read out register values
    success = twi_master_transfer((m_i2c_addr << 1) | TWI_READ_BIT, data_buffer, 2, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    *port0 = data_buffer[0];
    *port1 = data_buffer[1];

    return 0;
}

uint32_t app_pca9535a_input_state_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(0, port0, port1);
}

uint32_t app_pca9535a_output_state_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(2, port0, port1);
}

uint32_t app_pca9535a_port_config_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(6, port0, port1);
}

static uint32_t m_configure_ports(void)
{
    uint8_t data_buffer[BUF_LEN];
    bool success;

    // Set pin direction
    data_buffer[0] = 6;
    data_buffer[1] = 0xfb; // P0_2 is led0
    data_buffer[2] = 0xfb; // P1_2 is mpu9150 power control
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 3, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Set defaults
    data_buffer[0] = 2;
    data_buffer[1] = 0xff; // P0_2 is led0
    data_buffer[2] = 0xff;
    success = twi_master_transfer((m_i2c_addr << 1), data_buffer, 3, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    return 0;
}


uint32_t app_pca9535a_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return m_configure_ports();
}

