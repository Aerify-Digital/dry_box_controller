#ifndef LM75_H
#define LM75_H
#include <Arduino.h>
#include "hardware/i2c.h"

const uint LM75_BASE_ADDRESS = 0x48; // Base address 0b 1 0 0 1 A2 A1 A0
const uint LM75_REGISTER_TEMP = 0;   // Temperatur store register (RO)
const uint LM75_REGISTER_CONF = 1;   // Config register
const uint LM75_REGISTER_THYST = 2;  // Hysteresis register
const uint LM75_REGISTER_TOS = 3;    // Overtemperature shutdown register

const uint LM75_CONF_OS_COMP_INT = 1; // OS operation mode selection
const uint LM75_CONF_OS_POL = 2;      // OS polarity selection
const uint LM75_CONF_OS_F_QUE = 3;    // OS fault queue programming

// Write 1 byte to the specified register
int reg_write(i2c_inst_t *i2c,
              const uint addr,
              const uint8_t reg,
              uint8_t *buf,
              const uint8_t nbytes)
{

    int num_bytes_read = 0;
    uint8_t msg[nbytes + 1];

    // Check to make sure caller is sending 1 or more bytes
    if (nbytes < 1)
    {
        return 0;
    }

    // Append register address to front of data packet
    msg[0] = reg;
    for (int i = 0; i < nbytes; i)
    {
        msg[i + 1] = buf[i];
    }

    // Write data to register(s) over I2C
    i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

    return num_bytes_read;
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
int reg_read(i2c_inst_t *i2c,
             const uint addr,
             const uint8_t reg,
             uint8_t *buf,
             const uint8_t nbytes)
{

    int num_bytes_read = 0;

    // Check to make sure caller is asking for 1 or more bytes
    if (nbytes < 1)
    {
        return 0;
    }

    // Read data from register(s) over I2C
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

    return num_bytes_read;
}

#endif