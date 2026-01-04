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
#include "hardware/pwm.h"
#include "semphr.h"
#include <vector>
#include "debug.h"
#include "pindefs.h"
#include "menu.h"
#include "dht.h"
#include "eeprom.h"
#include "lcd.h"
#include "lm75.h"
#include "thermistor.h"

static QueueHandle_t usbQueue = NULL;
static QueueHandle_t lcdQueue = NULL;
static QueueHandle_t led1Queue = NULL;
static QueueHandle_t led2Queue = NULL;
static QueueHandle_t led3Queue = NULL;
static QueueHandle_t buzzerQueue = NULL;

static TaskHandle_t btnTaskHandle = NULL;
static TaskHandle_t ledTaskHandle = NULL;
static TaskHandle_t usbTaskHandle = NULL;
static TaskHandle_t lcdTaskHandle = NULL;
static TaskHandle_t obtTaskHandle = NULL;
static TaskHandle_t dht1TaskHandle = NULL;
static TaskHandle_t dht2TaskHandle = NULL;
static TaskHandle_t encoderTaskHandle = NULL;
static TaskHandle_t i2cScanTaskHandle = NULL;
static TaskHandle_t buzzerTaskHandle = NULL;
static TaskHandle_t thermistor1TaskHandle = NULL;
static TaskHandle_t thermistor2TaskHandle = NULL;

static SemaphoreHandle_t i2c_default_mutex;
static SemaphoreHandle_t adc_mutex;

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

typedef enum
{
    BUZZER_ON,
    BUZZER_OFF,
    BUZZER_CHIRP,
    BUZZER_ALARM
} BuzzType_t;

typedef struct
{
    BuzzType_t type;
    uint16_t duration_ms; // Only used for CHIRP
} BuzzerCommand_t;

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr)
{
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

std::string replace_placeholders(const char *src);

byte save_setting(uint8_t index, uint8_t value)
{
    return eeprom_write_byte(index, value);
}

void save_settings();

void load_settings();

void process_result(byte &result, const char *setting_name);