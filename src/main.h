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
#include <extEEPROM.h>

static QueueHandle_t usbQueue = NULL;
static QueueHandle_t lcdQueue = NULL;
static QueueHandle_t led1Queue = NULL;
static QueueHandle_t led2Queue = NULL;
static QueueHandle_t led3Queue = NULL;

static TaskHandle_t btnTaskHandle = NULL;
static TaskHandle_t ledTaskHandle = NULL;
static TaskHandle_t usbTaskHandle = NULL;
static TaskHandle_t lcdTaskHandle = NULL;
static TaskHandle_t obtTaskHandle = NULL;
static TaskHandle_t dht1TaskHandle = NULL;
static TaskHandle_t dht2TaskHandle = NULL;
static TaskHandle_t encoderTaskHandle = NULL;
static TaskHandle_t i2cScanTaskHandle = NULL;

static SemaphoreHandle_t i2c_default_mutex;
static SemaphoreHandle_t i2c1_mutex;

typedef enum
{
    LOG_NONE = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} LogLevel_t;

typedef struct
{
    char body[128];
    LogLevel_t level = LOG_NONE;
} Message_t;

static const int MSG_QUEUE_LEN = 64;

const uint EEPROM_ADDRESS = 0x50;

extEEPROM eeprom(kbits_64, 1, 32, EEPROM_ADDRESS);

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr)
{
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

std::string replace_placeholders(const char *src);

byte eeprom_write_byte(uint16_t mem_addr, uint8_t data)
{
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        byte result = eeprom.write(mem_addr, data);
        xSemaphoreGive(i2c1_mutex);
        return result;
    }
    else
    {
        Message_t msg;
        snprintf(msg.body, sizeof(msg.body), "eeprom_write_byte: Failed to obtain I2C1 mutex\n");
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        return 1; // Indicate failure
    }
}

uint8_t eeprom_read_byte(uint16_t mem_addr)
{
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        uint8_t data = eeprom.read(mem_addr);
        xSemaphoreGive(i2c1_mutex);
        return data;
    }
    else
    {
        Message_t msg;
        snprintf(msg.body, sizeof(msg.body), "eeprom_read_byte: Failed to obtain I2C1 mutex\n");
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        return 0; // Indicate failure
    }
}

void eeprom_write_buffer(uint16_t mem_addr, const uint8_t *data, size_t len)
{
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        eeprom.write(mem_addr, (byte *)data, len);
        xSemaphoreGive(i2c1_mutex);
    }
    else
    {
        Message_t msg;
        snprintf(msg.body, sizeof(msg.body), "eeprom_write_buffer: Failed to obtain I2C1 mutex\n");
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
}

void eeprom_read_buffer(uint16_t mem_addr, uint8_t *data, size_t len)
{
    if (xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        eeprom.read(mem_addr, (byte *)data, len);
        xSemaphoreGive(i2c1_mutex);
        return;
    }
    else
    {
        Message_t msg;
        snprintf(msg.body, sizeof(msg.body), "eeprom_read_buffer: Failed to obtain I2C1 mutex\n");
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        return;
    }
}

byte save_setting(uint8_t index, uint8_t value)
{
    byte result = eeprom_write_byte(index, value);
    return result;
}

void save_settings();

void load_settings();

void process_result(byte &result, const char *setting_name);