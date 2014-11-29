#include <stdbool.h>
#include "nrf.h"
#include "twi_master.h"
#include "app_mcp9808.h"
#include "app_pca9535a.h"
#include "app_max44009.h"

void uart_config(void)
{
    NRF_UART0->PSELTXD = 9;
    //NRF_UART0->PSELRXD = 11;

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud19200 << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled       << UART_ENABLE_ENABLE_Pos);
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

void uart_put_hex(uint8_t cr)
{
    static const char hex_tab[] = "0123456789ABCDEF";
    uart_put((uint8_t)hex_tab[(cr >>  4) & 0x0f]);
    uart_put((uint8_t)hex_tab[(cr >>  0) & 0x0f]);
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

void i2c_init()
{
}

void i2c_read_registers(uint8_t i2c_addr, uint8_t reg_addr, uint8_t len)
{
    uint8_t  i;
    bool     success;
    uint8_t  data[32] = {0};

    data[0] = reg_addr; // register address
    success = twi_master_transfer((i2c_addr << 1),                data,   1, TWI_DONT_ISSUE_STOP);
    if (! success) {
        uart_put_string("Addr wrong or register not found\n");
        return;
    }
    success = twi_master_transfer((i2c_addr << 1) | TWI_READ_BIT, data, len, TWI_ISSUE_STOP);
    if (! success) {
        uart_put_string("Read failed\n");
        return;
    }

    uart_put_string("  Data: 0x");
    for (i = 0; i < len; ++i) {
        uart_put_hex(data[i]);
    }
    uart_put_string("\n");
}

void i2c_write_registers(uint8_t i2c_addr, uint8_t reg_addr, uint8_t value)
{
    bool    success;
    uint8_t w2_data[2];

    w2_data[0] = reg_addr;
    w2_data[1] = value;
    success = twi_master_transfer((i2c_addr << 1), w2_data, 2, TWI_ISSUE_STOP);

    if (! success) {
        uart_put_string("Write failed\n");
    }
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
        uart_put_string("IOExt int\n");
    }

    if (NRF_GPIOTE->EVENTS_IN[1] == 1)
    {
        NRF_GPIOTE->EVENTS_IN[1] = 0;
        uart_put_string("Motion int\n");
    }
}

#define TEMP_SENSOR (0x1b)
#define IO_EXTENDER (0x27)
#define LIGHT_SENSOR (0x4a)

int main(void)
{
    uart_config();
    uart_put_string("####################################\n");

    twi_master_init();
    gpiote_init();

    {
        uint32_t err_code;
        int16_t  meas = 0;

        // Init module
        uart_put_string("Temperature\n");
        app_mcp9808_init(TEMP_SENSOR);

        // Power on chip, read measurement and go back to low power mode
        err_code = app_mcp9808_shutdown(false);
        if (err_code != 0) { uart_put_string("Fail\n"); }
        err_code = app_mcp9808_temp_read(&meas);
        if (err_code != 0) { uart_put_string("Fail\n"); }
        //err_code = app_mcp9808_shutdown(true);
        if (err_code != 0) { uart_put_string("Fail\n"); }

        // Print out result
        uart_put_string("  Degrees C: 0x");
        uart_put_hex((meas >>  8) & 0xff);
        uart_put_hex((meas >>  0) & 0xff);
        uart_put_string("\n");
    }

    {
        uint32_t err_code;
        uint8_t port0, port1;

        // Init module
        uart_put_string("IO Extender\n");
        err_code = app_pca9535a_init(IO_EXTENDER);
        if (err_code != 0) { uart_put_string("Fail\n"); }

        // Read port input state
        err_code = app_pca9535a_input_state_get(&port0, &port1);
        if (err_code != 0) { uart_put_string("Fail\n"); }
        uart_put_string("  Input port state:  0x");
        uart_put_hex(port1);
        uart_put_hex(port0);
        if ((port0 & BUTTON0_MASK) == 0) { uart_put_string(" button0"); }
        if ((port0 & BUTTON1_MASK) == 0) { uart_put_string(" button1"); }
        uart_put_string("\n");

        // Read port output state
        err_code = app_pca9535a_output_state_get(&port0, &port1);
        if (err_code != 0) { uart_put_string("Fail\n"); }
        uart_put_string("  Output port state: 0x");
        uart_put_hex(port1);
        uart_put_hex(port0);
        uart_put_string("\n");

        // Read polarity inversion state
        // TODO

        // Read port config
        err_code = app_pca9535a_port_config_get(&port0, &port1);
        if (err_code != 0) { uart_put_string("Fail\n"); }
        uart_put_string("  Port config:       0x");
        uart_put_hex(port1);
        uart_put_hex(port0);
        uart_put_string("\n");


        // Activate led0
        err_code = app_pca9535a_led0(true);
        if (err_code != 0) { uart_put_string("Fail\n"); }

        { uint32_t x = 0x0004ffff; while (--x != 0) { __NOP(); } }

        err_code = app_pca9535a_led0(false);
        if (err_code != 0) { uart_put_string("Fail\n"); }
    }

    {
        uint32_t err_code;
        uint8_t exponent, mantissa;

        // Init module
        uart_put_string("Light sensor\n");
        err_code = app_max44009_init(LIGHT_SENSOR);
        if (err_code != 0) { uart_put_string("Fail\n"); }

        // Read LUX value
        err_code = app_max44009_lux_read(&exponent, &mantissa);
        if (err_code != 0) { uart_put_string("Fail\n"); }

        // Print out result
        uart_put_string("  LUX: 2**0x");
        uart_put_hex(exponent);
        uart_put_string(" * 0x");
        uart_put_hex(mantissa);
        uart_put_string(" * 0.045\n");
    }

    {
        uart_put_string("Other\n");
        i2c_read_registers(0x5c, 0x09, 1);
        i2c_read_registers(0x5c, 0x0a, 1);
        i2c_read_registers(0x5c, 0x0f, 1);
        i2c_read_registers(0x5c, 0x10, 1);
        i2c_read_registers(0x5c, 0x20, 1);
        i2c_read_registers(0x5c, 0x21, 1);
        i2c_read_registers(0x5c, 0x22, 1);
        i2c_read_registers(0x5c, 0x23, 1);
        i2c_read_registers(0x5c, 0x24, 1);
        i2c_read_registers(0x5c, 0x25, 1);
        i2c_read_registers(0x5c, 0x27, 1);
        i2c_read_registers(0x5c, 0x28, 1);
        i2c_read_registers(0x5c, 0x27, 1);
        i2c_read_registers(0x5c, 0x29, 1);
        i2c_read_registers(0x5c, 0x2a, 1);
        i2c_read_registers(0x5c, 0x2b, 1);
        i2c_read_registers(0x5c, 0x2c, 1);
        i2c_read_registers(0x5c, 0x2e, 1);
        i2c_read_registers(0x5c, 0x2f, 1);
        i2c_read_registers(0x5c, 0x30, 1);
        i2c_read_registers(0x5c, 0x31, 1);
        i2c_read_registers(0x5c, 0x39, 1);
        i2c_read_registers(0x5c, 0x3a, 1);

        //i2c_write_registers(IO_EXTENDER, 6, 0xfb);
    }

    while (1) ;
}
