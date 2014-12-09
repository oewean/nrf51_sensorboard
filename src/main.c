#include <stdbool.h>
#include "nrf.h"
#include "hal_twi.h"
#include "app_mcp9808.h"
#include "app_pca9535a.h"
#include "app_max44009.h"
#include "app_lps25h.h"
#include "app_mpu9150.h"
#include "app_lis3dh.h"
#include "mpu6050.h"

static bool io_extender_irq_set = false;
static bool accel_irq_set = false;

void uart_config(void)
{
    NRF_UART0->PSELTXD = 9;
    //NRF_UART0->PSELRXD = 11;

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud1M << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled    << UART_ENABLE_ENABLE_Pos);
    NRF_UART0->TASKS_STARTTX = 1;
    //NRF_UART0->TASKS_STARTRX = 1;
    //NRF_UART0->EVENTS_RXDRDY = 0;
}

void uart_put(uint8_t cr)
{
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1) ;

    NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_put_hex_byte(uint8_t cr)
{
    static const char hex_tab[] = "0123456789ABCDEF";
    uart_put((uint8_t)hex_tab[(cr >>  4) & 0x0f]);
    uart_put((uint8_t)hex_tab[(cr >>  0) & 0x0f]);
}

void uart_put_hex_16(uint16_t value)
{
    uart_put_hex_byte((value >> 8) & 0xff);
    uart_put_hex_byte((value >> 0) & 0xff);
}

void uart_put_string(const uint8_t * str)
{
    uint_fast8_t i  = 0;
    uint8_t      ch = str[i++];

    while (ch != '\0')
    {
        uart_put(ch);
        ch = str[i++];
    }
}

void m_print_mpu9150_data(blapp * raw_values) {
    uart_put_string((const uint8_t *)" mpu9150: ");

    uart_put_hex_16(raw_values->value.x_accel);
    uart_put(',');
    uart_put_hex_16(raw_values->value.y_accel);
    uart_put(',');
    uart_put_hex_16(raw_values->value.z_accel);
    uart_put(';');
    uart_put(' ');

    uart_put_hex_16(raw_values->value.temperature);
    uart_put(';');
    uart_put(' ');

    uart_put_hex_16(raw_values->value.x_gyro);
    uart_put(',');
    uart_put_hex_16(raw_values->value.y_gyro);
    uart_put(',');
    uart_put_hex_16(raw_values->value.z_gyro);
    uart_put(';');
    uart_put(' ');

    uart_put_hex_16(raw_values->value.x_magn);
    uart_put(',');
    uart_put_hex_16(raw_values->value.y_magn);
    uart_put(',');
    uart_put_hex_16(raw_values->value.z_magn);

    //uart_put_string((const uint8_t *)"\r\n");
}

void i2c_read_registers(uint8_t i2c_addr, uint8_t reg_addr, uint8_t len)
{
    uint8_t  i;
    uint8_t  data[32] = {0};

    uart_put_string("  addr: 0x");
    uart_put_hex_byte(i2c_addr);
    uart_put_string(" reg_addr: 0x");
    uart_put_hex_byte(reg_addr);


    hal_twi_address_set(i2c_addr);
    hal_twi_stop_mode_set(HAL_TWI_STOP_MODE_STOP_ON_RX_BUF_END);

    data[0] = reg_addr; // register address
    hal_twi_write(1, data);
    //if (! success) {
    //    uart_put_string(" Error: addr wrong or register not found\n");
    //    return;
    //}
    hal_twi_read(len, data);
    //success = twi_master_transfer((i2c_addr << 1) | TWI_READ_BIT, data, len, TWI_ISSUE_STOP);
    //if (! success) {
    //    uart_put_string("  Read failed\n");
    //    return;
    //}

    uart_put_string(" data: 0x");
    for (i = 0; i < len; ++i) {
        uart_put_hex_byte(data[i]);
    }
    uart_put_string("\n");
}

void i2c_write_register(uint8_t i2c_addr, uint8_t reg_addr, uint8_t value)
{
    uint8_t w2_data[2];

    w2_data[0] = reg_addr;
    w2_data[1] = value;
    hal_twi_write(2, w2_data);

    //if (! success) {
    //    uart_put_string("  Write failed\n");
    //}
}


bool twi_master_transfer(uint8_t   address, uint8_t * data, uint8_t   data_length, bool      issue_stop_condition)
{

    return false;
}

static void gpiote_init(void)
{
    NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos)     |
                            (17                            << GPIOTE_CONFIG_PSEL_Pos)     |
                            (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);

    NRF_GPIOTE->CONFIG[1] = (GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos)     |
                            (18                            << GPIOTE_CONFIG_PSEL_Pos)     |
                            (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);

    /* Three NOPs are required to make sure configuration is written before setting tasks or getting events */
    __NOP();
    __NOP();
    __NOP();

    /* Clear the event that appears in some cases */
    NRF_GPIOTE->EVENTS_IN[0] = 0;
    NRF_GPIOTE->EVENTS_IN[1] = 0;

    // Enable interrupt on input 1 event.
    NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN0_Enabled << GPIOTE_INTENSET_IN0_Pos);
    NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN1_Enabled << GPIOTE_INTENSET_IN1_Pos);

    NVIC_EnableIRQ(GPIOTE_IRQn);
}

void GPIOTE_IRQHandler(void)
{
    if (NRF_GPIOTE->EVENTS_IN[0] == 1)
    {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
        accel_irq_set = true;
    }

    if (NRF_GPIOTE->EVENTS_IN[1] == 1)
    {
        NRF_GPIOTE->EVENTS_IN[1] = 0;
        io_extender_irq_set = true;
    }
}

#define TEMP_SENSOR (0x1b)
#define IO_EXTENDER (0x27)
#define LIGHT_SENSOR (0x4a)
#define PRESSURE_SENSOR (0x5c)
#define MOTION_TRACKER (0x68)
#define ACCELEROMETER (0x18)

int main(void)
{
    uart_config();
    uart_put_string("####################################\n");

    //twi_master_init();
    hal_twi_init();
    gpiote_init();

    //NRF_GPIO->DIRSET = 0xff7c0000;
    //NRF_GPIO->OUTCLR = 0xff7c0000;

    //{
    //    uint32_t err_code;
    //    int16_t  temperature = 0;

    //    // Init module
    //    uart_put_string("Temperature\n");
    //    app_mcp9808_init(TEMP_SENSOR);

    //    // Power on chip, read measurement and go back to low power mode
    //    err_code = app_mcp9808_shutdown(false);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    err_code = app_mcp9808_temp_read(&temperature);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    //err_code = app_mcp9808_shutdown(true);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Print out result
    //    uart_put_string("  Degrees C: 0x");
    //    uart_put_hex_byte((temperature >>  8) & 0xff);
    //    uart_put_hex_byte((temperature >>  0) & 0xff);
    //    uart_put_string(" / 16.0\n");
    //}

    //{
    //    uint32_t err_code;
    //    uint8_t port0, port1;

    //    // Init module
    //    uart_put_string("IO Extender\n");
    //    err_code = app_pca9535a_init(IO_EXTENDER);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Read port input state
    //    err_code = app_pca9535a_input_state_get(&port0, &port1);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    uart_put_string("  Input port state:  0x");
    //    uart_put_hex_byte(port1);
    //    uart_put_hex_byte(port0);
    //    if ((port0 & BUTTON0_MASK) == 0) { uart_put_string(" button0"); }
    //    if ((port0 & BUTTON1_MASK) == 0) { uart_put_string(" button1"); }
    //    uart_put_string("\n");

    //    // Read port output state
    //    err_code = app_pca9535a_output_state_get(&port0, &port1);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    uart_put_string("  Output port state: 0x");
    //    uart_put_hex_byte(port1);
    //    uart_put_hex_byte(port0);
    //    uart_put_string("\n");

    //    // Read polarity inversion state
    //    // TODO

    //    // Read port config
    //    err_code = app_pca9535a_port_config_get(&port0, &port1);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    uart_put_string("  Port config:       0x");
    //    uart_put_hex_byte(port1);
    //    uart_put_hex_byte(port0);
    //    uart_put_string("\n");


    //    // Activate led0
    //    err_code = app_pca9535a_led0(true);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    { uint32_t x = 0x0004ffff; while (--x != 0) { __NOP(); } }

    //    err_code = app_pca9535a_led0(false);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //}

    //{
    //    uint32_t err_code;
    //    uint8_t exponent, mantissa;

    //    // Init module
    //    uart_put_string("Light sensor\n");
    //    err_code = app_max44009_init(LIGHT_SENSOR);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Read LUX value
    //    err_code = app_max44009_lux_read(&exponent, &mantissa);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Print out result
    //    uart_put_string("  LUX: 2**0x");
    //    uart_put_hex_byte(exponent);
    //    uart_put_string(" * 0x");
    //    uart_put_hex_byte(mantissa);
    //    uart_put_string(" * 0.045\n");
    //}

    //{
    //    uint32_t err_code;
    //    int32_t pressure;
    //    int16_t temperature;

    //    // Init module
    //    uart_put_string("Pressure sensor\n");
    //    err_code = app_lps25h_init(PRESSURE_SENSOR);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Read pressure value
    //    err_code = app_lps25h_press_read(&pressure);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Read temperature value
    //    err_code = app_lps25h_temp_read(&temperature);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Print out result
    //    uart_put_string("  hPa: 0x");
    //    uart_put_hex_byte((pressure >> 16) & 0xff);
    //    uart_put_hex_byte((pressure >>  8) & 0xff);
    //    uart_put_hex_byte((pressure >>  0) & 0xff);
    //    uart_put_string(" / 4096.0\n");

    //    uart_put_string("  Degrees C: 42.5 * (0x");
    //    uart_put_hex_byte((temperature >>  8) & 0xff);
    //    uart_put_hex_byte((temperature >>  0) & 0xff);
    //    uart_put_string(" / 480.0) --  TODO: These registers typically read out as 0x0000. Wtf?\n");
    //}

    //{
    //    uint32_t err_code;
    //    int16_t temperature;

    //    uart_put_string("Motion tracking\n");

    //    // Power up and test
    //    i2c_write_register(0x27, 0x07, 0xfb);
    //    i2c_write_register(0x27, 0x03, 0xff);
    //    i2c_read_registers(0x68, 0x75, 1); // who am I

    //    err_code = mpu6050_init(0x68);
    //    // TODO: Add delays to _init. Causes problems
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    //// Set sample rate
    //    //err_code = mpu6050_write(MPU6050_RA_CONFIG, 6);
    //    //CHECK_SUCCESS(err_code);

    //    //// Set sample rate divider
    //    //err_code = mpu6050_write(MPU6050_RA_SMPLRT_DIV, 9);
    //    //CHECK_SUCCESS(err_code);

    //    // Set motion detection threshold
    //    err_code = mpu6050_write(MPU6050_RA_MOT_THR, 5);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    err_code = mpu6050_write(MPU6050_RA_MOT_DUR, 2);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Set zero motion detection threshold
    //    err_code = mpu6050_write(MPU6050_RA_ZRMOT_THR, 120);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    err_code = mpu6050_write(MPU6050_RA_ZRMOT_DUR, 120);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Enable data ready interrupt
    //    //uint8_t int_value = ( MPU6050_INTERRUPT_FF_MASK
    //    //                    | MPU6050_INTERRUPT_MOT_MASK
    //    //                    | MPU6050_INTERRUPT_ZRMOT_MASK
    //    //                    | MPU6050_INTERRUPT_DATA_RDY_MASK);
    //    uint8_t int_value = ( MPU6050_INTERRUPT_MOT_MASK );
    //    err_code = mpu6050_write(MPU6050_RA_INT_ENABLE, int_value);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    { uint32_t x = 0x0004ffff; while (--x != 0) { __NOP(); } }

    //    err_code = mpu6050_temp_read(&temperature);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    uart_put_string("  Degrees C: 35 + (0x");
    //    uart_put_hex_byte((temperature >>  8) & 0xff);
    //    uart_put_hex_byte((temperature >>  0) & 0xff);
    //    uart_put_string(" / 340.0)\n");

    //    blapp raw_values;
    //    err_code = mpu6050_raw_sensor_read(&raw_values);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }
    //    uart_put_string(" ");
    //    m_print_mpu9150_data(&raw_values);
    //    uart_put_string("\n");


    //    //// Init module
    //    //err_code = app_mpu9150_init(MOTION_TRACKER);
    //    //if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    //// Testing
    //    //i2c_read_registers(0x68, 0x6b, 1);
    //    ////i2c_write_register(0x68, 0x68, 0x01);
    //    ////i2c_read_registers(0x68, 0x68, 1);

    //    //{ uint32_t x = 0x000fffff; while (--x != 0) { __NOP(); } }

    //    //i2c_read_registers(0x68, 0x03, 6); // magnet
    //    //i2c_read_registers(0x68, 0x59, 6); // accel
    //    //i2c_read_registers(0x68, 0x65, 2); // temp
    //    //i2c_read_registers(0x68, 0x67, 6); // gyro
    //    //i2c_read_registers(0x68, 0x75, 1); // who am I

    //    //// Read TODO: sadf

    //    //// De-activate mpu9150
    //    //uart_put_string("  Deactivate mpu9150\n");
    //    //i2c_read_registers(IO_EXTENDER, 0x06, 2);
    //    //i2c_read_registers(IO_EXTENDER, 0x02, 2);
    //    ////i2c_write_register(IO_EXTENDER, 0x03, 0xff);
    //    //i2c_read_registers(IO_EXTENDER, 0x02, 2);
    //}

    //{
    //    uint32_t err_code;

    //    // Init module
    //    uart_put_string("Accelerometer\n");
    //    err_code = app_lis3dh_init(ACCELEROMETER);
    //    if (err_code != 0) { uart_put_string("  Fail\n"); }

    //    // Testing
    //    //i2c_read_registers(0x68, 0x09, 1);
    //    i2c_read_registers(0x18, 0x28, 6);

    //    // Read TODO: sadf
    //}

    {
        uart_put_string("Other\n");
        i2c_read_registers(0xff, 0xff, 1);

        //i2c_read_registers(PRESSURE_SENSOR, 0x0f, 1);
        //i2c_read_registers(MOTION_TRACKER, 0x75, 1);

        //i2c_write_register(IO_EXTENDER, 6, 0xfb);
    }

    //uart_put_string("while(1)\n");
    //while (1)
    //{
    //    uint32_t err_code;

    //    if (io_extender_irq_set)
    //    {
    //        uint8_t port0, port1;

    //        io_extender_irq_set = false;

    //        err_code = app_pca9535a_input_state_get(&port0, &port1);
    //        if (err_code != 0) { uart_put_string("  Fail\n"); }
    //        uart_put_string("  Input port state:  0x");
    //        uart_put_hex_byte(port1);
    //        uart_put_hex_byte(port0);

    //        if ((port1 & (1 << 5)) == 0) {
    //            blapp raw_values;
    //            err_code = mpu6050_raw_sensor_read(&raw_values);
    //            if (err_code != 0) { uart_put_string("  Fail\n"); }
    //            m_print_mpu9150_data(&raw_values);
    //        }

    //        if ((port0 & BUTTON0_MASK) == 0) { uart_put_string(" button0"); }
    //        if ((port0 & BUTTON1_MASK) == 0) { uart_put_string(" button1"); }

    //        uart_put_string("\n");
    //    }
    //    if (accel_irq_set)
    //    {
    //        accel_irq_set = false;
    //        uart_put_string("Accel int\n");
    //    }
    //}
}
