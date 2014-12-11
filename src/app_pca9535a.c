#include "app_pca9535a.h"
#include "hal_twi.h"

#define BUF_LEN (32)    // TODO: Find actual size
static uint8_t m_i2c_addr;

uint32_t m_read_register_pairs(uint8_t reg_addr, uint8_t * port0, uint8_t * port1)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);

    // Select register pair for reading
    data[0] = reg_addr;
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

    *port0 = data[0];
    *port1 = data[1];

    return 0;

}

uint32_t app_pca9535a_input_state_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(PCA9535A_RA_INPUT_0, port0, port1);
}

uint32_t app_pca9535a_output_state_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(PCA9535A_RA_OUTPUT_0, port0, port1);
}

uint32_t app_pca9535a_port_config_get(uint8_t * port0, uint8_t * port1)
{
    return m_read_register_pairs(PCA9535A_RA_CONFIG_0, port0, port1);
}

uint32_t app_pca9535a_led0(bool on)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    // Read status
    err_code = m_read_register_pairs(PCA9535A_RA_OUTPUT_0, &data[1], &data[2]);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_TX_BUF_END);

    // Write led bit in config register
    data[0]  = PCA9535A_RA_OUTPUT_0;
    data[1] = (data[1] & (0xff ^ (1 << 2))) | ( on ? 0 : (1 << 2) );
    err_code = hal_twi_write(2, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    return 0;
}

static uint32_t m_configure_ports(void)
{
    uint8_t data[BUF_LEN];
    uint32_t err_code;

    hal_twi_address_set(m_i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_TX_BUF_END);

    // Set pin direction
    data[0] = PCA9535A_RA_CONFIG_0;
    data[1] = 0xfb; // P0_2 is led0
    data[2] = 0xfb; // P1_2 is mpu9150 power control

    err_code = hal_twi_write(3, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }

    // Set defaults
    data[0] = PCA9535A_RA_OUTPUT_0;
    data[1] = 0xff; // P0_2 is led0
    data[2] = 0xff;

    err_code = hal_twi_write(3, data);
    if (err_code != HAL_TWI_STATUS_CODE_SUCCESS)
    {
        return err_code;
    }


    return 0;
}


uint32_t app_pca9535a_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return m_configure_ports();
}

