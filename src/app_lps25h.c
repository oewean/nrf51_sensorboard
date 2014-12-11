#include "app_lps25h.h"
#include "twi_master.h"

#define BUF_LEN            (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = 0;

/* TODO: Set suggested default low power settings from documentation:
 * url: http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00066332.pdf
 *
 * 9.1 Filter enabling and suggested configuration
 *     To reduce the internal pressure and temperature average the
 *     configuration below can be used:
 *     - RES_CONF (10h) = 05h
 *     - FIFO_CTRL (2Eh) = C0
 *     - CTRL_REG2 (21h) = 40h
 *     In this way, the power consumption at 1 Hz is reduced from 25 µA
 *     (typical) to 4.5 µA (typical) with a pressure noise of 0.01 hPa rms
 */

static uint32_t m_reg_read(uint8_t reg, uint8_t * data)
{
    bool success;

    // Select register for reading
    success = twi_master_transfer((m_i2c_addr << 1), &reg, 1, TWI_DONT_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Read out register value
    success = twi_master_transfer((m_i2c_addr << 1) | TWI_READ_BIT, data, 1, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    return 0;
}

uint32_t app_lps25h_press_read(int32_t * p_meas)
{
    uint8_t reg_value;
    uint32_t err_code;

    // Read PRESS_OUT_XL
    err_code = m_reg_read(0x28, &reg_value);
    if (err_code != 0) return err_code;
    *p_meas = reg_value;

    // Read PRESS_OUT_L
    err_code = m_reg_read(0x29, &reg_value);
    if (err_code != 0) return err_code;
    *p_meas |= (reg_value << 8);

    // Read PRESS_OUT_H
    err_code = m_reg_read(0x2a, &reg_value);
    if (err_code != 0) return err_code;
    *p_meas |= (reg_value << 16);

    // Padd if necessary to 32 bit, 2's compliment
    if ((reg_value & 0x80) == 0x80) // TODO: Verify this
    {
        *p_meas |= 0xff000000;
    }

    return 0;
}

uint32_t app_lps25h_temp_read(int16_t * p_meas)
{
    uint8_t reg_value;
    uint32_t err_code;

    // Read TEMP_OUT_L
    err_code = m_reg_read(0x2b, &reg_value);
    if (err_code != 0) return err_code;
    *p_meas = reg_value;

    // Read TEMP_OUT_H
    err_code = m_reg_read(0x2c, &reg_value);
    if (err_code != 0) return err_code;
    *p_meas |= (reg_value << 8);

    return 0;
}


static uint32_t m_check_who_am_i(void)
{
    uint8_t data[BUF_LEN];
    bool success;

    // Select who am I register for reading
    data[0] = 0x0f;
    success = twi_master_transfer((m_i2c_addr << 1), data, 1, TWI_DONT_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    // Read out register values
    success = twi_master_transfer((m_i2c_addr << 1) | TWI_READ_BIT, data, 1, TWI_ISSUE_STOP);
    if (! success)
    {
        return 1;
    }

    if (data[0] != 0xbd)
    {
        return 2;
    }

    return 0;
}

uint32_t app_lps25h_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return m_check_who_am_i();
}

