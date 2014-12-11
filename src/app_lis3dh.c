#include "app_lis3dh.h"
#include "hal_twi.h"

#define BUF_LEN (32)    // TODO: Find actual size
static uint8_t m_i2c_addr;

static uint32_t m_check_who_am_i(void)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    // Select who am I register for reading
    data[0] = 0x0f;
    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);
    err_code = hal_twi_write(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return 15;
    }

    // Read out register values
    err_code = hal_twi_read(1, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return 16;
    }

    if (data[0] != 0x33)
    {
        return 14;
    }

    return 0;
}

uint32_t app_lis3dh_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return m_check_who_am_i();
}

