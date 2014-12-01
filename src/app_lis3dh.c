#include "app_lis3dh.h"
#include <stdlib.h>
//#include <string.h>
#include "twi_master.h"

#define BUF_LEN             (32)    // TODO: Find actual size
static uint8_t m_i2c_addr = NULL;

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

    if (data[0] != 0x33)
    {
        return 2;
    }

    return 0;
}

uint32_t app_lis3dh_init(uint8_t i2c_addr)
{
    m_i2c_addr = i2c_addr;

    return m_check_who_am_i();
}

