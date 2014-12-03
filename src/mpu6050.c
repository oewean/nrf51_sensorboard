#include <stdbool.h>
#include <stdint.h>

#include "twi_master.h"
#include "mpu6050.h"

static const uint8_t expected_who_am_i = 0x68U;
static uint8_t       m_device_address;

static bool mpu6050_verify_product_id(void)
{
    uint8_t who_am_i;

    if (mpu6050_read(MPU6050_RA_WHO_AM_I, &who_am_i, 1) != 0) {
        return false;
    }

    if (who_am_i != expected_who_am_i) {
        return false;
    } else {
        return true;
    }
}

uint32_t mpu6050_init(uint8_t device_address)
{
    uint32_t err_code;
    m_device_address = (uint8_t)(device_address << 1);

    // Check that the mpu6050 is responding as expected
    if (! mpu6050_verify_product_id()) {
        return 1;
    }

    // Reset signal path
    uint8_t reset_value = ( (1UL << MPU6050_PATHRESET_GYRO_RESET_BIT) 
                          | (1UL << MPU6050_PATHRESET_ACCEL_RESET_BIT)
                          | (1UL << MPU6050_PATHRESET_TEMP_RESET_BIT));
    err_code = mpu6050_write(MPU6050_RA_SIGNAL_PATH_RESET, reset_value);
    if (err_code != 0) {
        return 1;
    }

    // Go out of sleep if sleeping
    err_code = mpu6050_sleep_set(false);
    if (err_code != 0) {
        return 1;
    }

    return 0;
}

uint32_t mpu6050_write(uint8_t register_address, uint8_t value)
{
    uint8_t w2_data[2];

    w2_data[0] = register_address;
    w2_data[1] = value;
    if (! twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP)) {
        return 1;
    }
    return 0;
}

uint32_t mpu6050_read(uint8_t register_address, uint8_t * destination, uint8_t number_of_bytes)
{
    if (! twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP)) {
        return 1;
    }
    if (! twi_master_transfer(m_device_address|TWI_READ_BIT, destination, number_of_bytes, TWI_ISSUE_STOP)) {
        return 1;
    }
    return 0;
}

static inline void swap(uint8_t * h, uint8_t * l) {
    uint8_t s;
    s = *l;
    *l = *h;
    *h = s;
}

uint32_t mpu6050_raw_sensor_read(blapp * sensor_data)
{
    uint32_t err_code;

    err_code = mpu6050_read(MPU6050_RA_FIRST_SENSOR_REG, (uint8_t*)sensor_data, 14);
    if (err_code != 0) {
        return 1;
    }

    err_code = mpu6050_read(0x03, &sensor_data->reg.x_magn_h, 6);
    if (err_code != 0) {
        return 1;
    }

    if (err_code == 0) {
        swap(&sensor_data->reg.x_accel_h, &sensor_data->reg.x_accel_l);
        swap(&sensor_data->reg.y_accel_h, &sensor_data->reg.y_accel_l);
        swap(&sensor_data->reg.z_accel_h, &sensor_data->reg.z_accel_l);

        swap(&sensor_data->reg.t_h, &sensor_data->reg.t_l);

        swap(&sensor_data->reg.x_gyro_h, &sensor_data->reg.x_gyro_l);
        swap(&sensor_data->reg.y_gyro_h, &sensor_data->reg.y_gyro_l);
        swap(&sensor_data->reg.z_gyro_h, &sensor_data->reg.z_gyro_l);

        swap(&sensor_data->reg.x_magn_h, &sensor_data->reg.x_magn_l);
        swap(&sensor_data->reg.y_magn_h, &sensor_data->reg.y_magn_l);
        swap(&sensor_data->reg.z_magn_h, &sensor_data->reg.z_magn_l);
    }

    return 0;
}

uint32_t mpu6050_temp_read(int16_t * p_meas)
{
    uint8_t data[2];
    uint32_t err_code;

    err_code = mpu6050_read(MPU6050_RA_TEMP_OUT_H, (uint8_t*)data, sizeof(data));
    if (err_code != 0) {
        return 1;
    }

    *p_meas = data[0] << 8 | data[1];

    return 0;
}

uint32_t mpu6050_sleep_set(bool sleep)
{
    uint32_t err_code;
    if (sleep) {
        err_code = mpu6050_write(0x6b, 0x40);
    } else {
        err_code = mpu6050_write(0x6b, 0);
    }
    return err_code;
}

