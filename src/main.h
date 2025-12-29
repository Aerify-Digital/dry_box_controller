#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "semphr.h"
#include <vector>
#include "debug.h"
#include "pindefs.h"
#include "menu.h"
#include "dht.h"
#include "lcd.h"
#include "lm75.h"

typedef struct
{
    char body[128];
} Message_t;

static const int MSG_QUEUE_LEN = 64;

const uint EEPROM_ADDRESS = 0x50;

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr)
{
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}