#ifndef EEPROM_H
#define EEPROM_H
#include <Arduino.h>
#include <extEEPROM.h>

const uint EEPROM_ADDRESS = 0x50;

extEEPROM eeprom(kbits_64, 1, 32, EEPROM_ADDRESS);

byte eeprom_write_byte(uint16_t mem_addr, uint8_t data)
{
    return eeprom.write(mem_addr, data);
}

uint8_t eeprom_read_byte(uint16_t mem_addr)
{
    return eeprom.read(mem_addr);
}

byte eeprom_write_buffer(uint16_t mem_addr, const uint8_t *data, size_t len)
{
    return eeprom.write(mem_addr, (byte *)data, len);
}

byte eeprom_read_buffer(uint16_t mem_addr, uint8_t *data, size_t len)
{
    return eeprom.read(mem_addr, (byte *)data, len);
}

#endif