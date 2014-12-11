#include "app_mpu9150.h"
#include <stdbool.h>
#include "twi_master.h"

#define BUF_LEN (32)    // TODO: Find actual size
static uint8_t m_i2c_addr;

static bool m_check_who_am_i(void)
{
    uint8_t data[BUF_LEN];

    // Select who am I register for reading
    data[0] = 0x75;
    if (! twi_master_transfer((m_i2c_addr << 1), data, 1, TWI_DONT_ISSUE_STOP) )
    {
        return false;
    }

    // Read out register values
    if (! twi_master_transfer((m_i2c_addr << 1) | TWI_READ_BIT, data, 1, TWI_ISSUE_STOP) )
    {
        return false;
    }

    return data[0] == 0x68;
}

uint32_t app_mpu9150_sleep_set(bool sleep)
{
    uint8_t data[BUF_LEN];

    data[0] = 0x6b; // Select register

    // TODO: Read out value first, only flip single bit
    if (sleep) {
        data[1] = 0x40;
    } else {
        data[1] = 0x00;
    }

    if (! twi_master_transfer((m_i2c_addr << 1), data, 2, TWI_ISSUE_STOP) )
    {
        return 1;
    }

    return 0;
}

static bool m_reset_signal_path(void)
{
    uint8_t data[BUF_LEN];

    data[0] = 0x68;          // Select register
    data[1] = ( (1UL << 0)   //MPU6050_PATHRESET_GYRO_RESET_BIT)  // TODO: Fix magic numbers
              | (1UL << 1)   //MPU6050_PATHRESET_ACCEL_RESET_BIT)
              | (1UL << 2)); //MPU6050_PATHRESET_TEMP_RESET_BIT));

    if (! twi_master_transfer((m_i2c_addr << 1), data, 2, TWI_ISSUE_STOP) )
    {
        return false;
    }

    return true;
}

uint32_t app_mpu9150_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    if (! m_check_who_am_i() )
    {
        return 1;
    }

    // Reset signal path
    if (! m_reset_signal_path() )
    {
        return 1;
    }

    // Go out of sleep if sleeping
    return app_mpu9150_sleep_set(false);
}

