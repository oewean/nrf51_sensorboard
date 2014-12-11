#include "app_lps25h.h"
#include "hal_twi.h"

#define BUF_LEN (32)    // TODO: Find actual size
static uint8_t m_i2c_addr;

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
    uint32_t err_code;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);

    // Select register for reading
    err_code = hal_twi_write(1, &reg);
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
    uint32_t err_code;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);

    // Select who am I register for reading
    data[0] = 0x0f;
    err_code = hal_twi_write(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Read out register values
    err_code = hal_twi_read(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
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

