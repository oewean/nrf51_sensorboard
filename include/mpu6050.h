/* ============================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2012 Jeff Rowberg

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
*/

/* Note: This header file is just a small subset of the original I2Cdev mpu6050
 * library from Jeff Rowberg (--vegarwe) */

#ifndef MPU6050_H
#define MPU6050_H

#include <stdbool.h>
#include <stdint.h>

#define MPU6050_RA_SMPLRT_DIV               0x19
#define MPU6050_RA_CONFIG                   0x1A
#define MPU6050_RA_MOT_THR                  0x1F
#define MPU6050_RA_MOT_DUR                  0x20
#define MPU6050_RA_ZRMOT_THR                0x21
#define MPU6050_RA_ZRMOT_DUR                0x22
#define MPU6050_RA_INT_ENABLE               0x38
#define MPU6050_RA_INT_STATUS               0x3A
#define MPU6050_RA_FIRST_SENSOR_REG         0x3B
#define MPU6050_RA_TEMP_OUT_H               0x41
#define MPU6050_RA_SIGNAL_PATH_RESET        0x68
#define MPU6050_RA_WHO_AM_I                 0x75

#define MPU6050_PATHRESET_GYRO_RESET_BIT    2
#define MPU6050_PATHRESET_ACCEL_RESET_BIT   1
#define MPU6050_PATHRESET_TEMP_RESET_BIT    0

#define MPU6050_INTERRUPT_FF_BIT            7
#define MPU6050_INTERRUPT_MOT_BIT           6
#define MPU6050_INTERRUPT_ZRMOT_BIT         5
#define MPU6050_INTERRUPT_DMP_INT_BIT       1
#define MPU6050_INTERRUPT_DATA_RDY_BIT      0
#define MPU6050_INTERRUPT_FF_MASK           (1UL << MPU6050_INTERRUPT_FF_BIT)
#define MPU6050_INTERRUPT_MOT_MASK          (1UL << MPU6050_INTERRUPT_MOT_BIT)
#define MPU6050_INTERRUPT_ZRMOT_MASK        (1UL << MPU6050_INTERRUPT_ZRMOT_BIT)
#define MPU6050_INTERRUPT_DMP_INT_MASK      (1UL << MPU6050_INTERRUPT_DMP_INT_BIT)
#define MPU6050_INTERRUPT_DATA_RDY_MASK     (1UL << MPU6050_INTERRUPT_DATA_RDY_BIT)

#define MPU6050_NRF51_INTERRUPT_PIN         17
#define MPU6050_NRF51_INTERRUPT_MASK        (1UL << MPU6050_NRF51_INTERRUPT_PIN)

typedef union
{
    struct
    {
        uint8_t x_accel_h;
        uint8_t x_accel_l;
        uint8_t y_accel_h;
        uint8_t y_accel_l;
        uint8_t z_accel_h;
        uint8_t z_accel_l;
        uint8_t t_h;
        uint8_t t_l;
        uint8_t x_gyro_h;
        uint8_t x_gyro_l;
        uint8_t y_gyro_h;
        uint8_t y_gyro_l;
        uint8_t z_gyro_h;
        uint8_t z_gyro_l;
        uint8_t x_magn_h;
        uint8_t x_magn_l;
        uint8_t y_magn_h;
        uint8_t y_magn_l;
        uint8_t z_magn_h;
        uint8_t z_magn_l;
    } reg;
    struct
    {
        int16_t x_accel;
        int16_t y_accel;
        int16_t z_accel;
        int16_t temperature;
        int16_t x_gyro;
        int16_t y_gyro;
        int16_t z_gyro;
        int16_t x_magn;
        int16_t y_magn;
        int16_t z_magn;
    } value;
} blapp;

uint32_t mpu6050_init(uint8_t device_address);

uint32_t mpu6050_write(uint8_t register_address, const uint8_t value);

uint32_t mpu6050_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes);

uint32_t mpu6050_temp_read(int16_t * p_meas);

uint32_t mpu6050_raw_sensor_read(blapp * sensor_data);

uint32_t mpu6050_sleep_set(bool sleep);

#endif /* MPU6050_H */
