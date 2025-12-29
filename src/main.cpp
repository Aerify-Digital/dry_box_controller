#include "main.h"

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

int enc_a = 1;
int enc_a_last = 1;
int enc_b = 1;
float board_temp = 20;
uint menu_item = 0;
int m = 0;
int menu = -1;

DHT_Data_t th1 = {0, 8, 100.0, 20.0};
DHT_Data_t th2 = {1, 9, 100.0, 20.0};
std::vector<DHT_Data_t> dht_readings = std::vector<DHT_Data_t>(2);

volatile uint32_t last_button_press = 0;
volatile bool button_event = false;

void gpio_callback(uint gpio, uint32_t events)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_button_press < 200)
    {
        return;
    }
    last_button_press = now;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    switch (gpio)
    {
    case 14:
        button_event = true;
        break;
    default:
        break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_task(void *pvParameters)
{
    while (1)
    {
        if (button_event)
        {
            button_event = false;

            switch (menu)
            {
            case MAIN_MENU:
                switch (menu_item)
                {
                case HARDWARE_BTN:
                    menu = HARDWARE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case HUMIDITY_BTN:
                    menu = HUMIDITY_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case TEMPERATURE_BTN:
                    menu = TEMPERATURE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                default:
                    // info screen
                    menu = -1;
                    menu_item = 0;
                    m = 0;
                    xQueueSend(lcdQueue, (void *)true, 0);
                    break;
                }
                break;
            case HARDWARE_MENU:
                switch (menu_item)
                {
                case DHT_BTN:
                    menu = DHT_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                    break;
                case THERMAL_BTN:
                    menu = THERMAL_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                    break;
                case OUTPUT_BTN:
                    menu = OUTPUT_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case SPEAKER_BTN:
                    menu = SPEAKER_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                default:
                    menu = MAIN_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case DHT_MENU:
                switch (menu_item)
                {
                default:
                    menu = HARDWARE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case THERMAL_MENU:
                switch (menu_item)
                {
                default:
                    menu = HARDWARE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case OUTPUT_MENU:
                switch (menu_item)
                {
                default:
                    menu = HARDWARE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case SPEAKER_MENU:
                switch (menu_item)
                {
                default:
                    menu = HARDWARE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case HUMIDITY_MENU:
                switch (menu_item)
                {
                case 1:
                    menu = HMIN_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case 2:
                    menu = HMAX_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case 3:
                    menu = HTARGET_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                default:
                    menu = MAIN_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case TEMPERATURE_MENU:
                switch (menu_item)
                {
                case 1:
                    menu = TMIN_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case 2:
                    menu = TMAX_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case 3:
                    menu = TTARGET_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                case 4:
                    menu = UNIT_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                default:
                    menu = MAIN_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            default:
                menu = MAIN_MENU;
                menu_item = 0;
                m = 0;
                break;
            }
            xQueueSend(lcdQueue, (void *)true, 0);
        }
        vTaskDelay(1);
    }
}

void led_task(void *pvParameters)
{

    pinMode(LED_1_PIN, GPIO_OUT);
    pinMode(LED_2_PIN, GPIO_OUT);
    pinMode(LED_3_PIN, GPIO_OUT);
    bool rcv_val1;
    bool rcv_val2;
    bool rcv_val3;

    Message_t msg;
    snprintf(msg.body, 128, "LED Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    while (1)
    {
        xQueueReceive(led1Queue, &rcv_val1, portMAX_DELAY);
        xQueueReceive(led2Queue, &rcv_val2, portMAX_DELAY);
        xQueueReceive(led3Queue, &rcv_val3, portMAX_DELAY);
        gpio_put(LED_1_PIN, rcv_val1);
        gpio_put(LED_2_PIN, rcv_val2);
        gpio_put(LED_3_PIN, rcv_val3);
        vTaskDelay(1);
    }
}

void usb_task(void *pvParameters)
{

    Serial.printf("USB Task Started\n");
    Message_t rcv_msg;
    while (1)
    {
        xQueueReceive(usbQueue, (void *)&rcv_msg, portMAX_DELAY);
        Serial.printf("%s", rcv_msg.body);
        vTaskDelay(1);
    }
}

void lcd_task(void *pvParameters)
{

    Message_t msg;
    snprintf(msg.body, 128, "LCD Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    lcd_init();
    bool update = false;
    xQueueSend(lcdQueue, (void *)true, 10);
    while (1)
    {
        xQueueReceive(lcdQueue, (void *)&update, portMAX_DELAY);
        if (xSemaphoreTake(i2c_default_mutex, 1000) == pdTRUE)
        {
            if (update)
            {
                if (menu == -1)
                {

                    double temp = dht_readings.at(0).celsius;
                    double hum = dht_readings.at(0).humidity;
                    char temp_str[17];
                    snprintf(temp_str, 17, "%6.1fC  %-5.1f%% ", temp, hum);
                    lcd_set_cursor(0, 0);
                    lcd_string("  TEMP     HUM  ");
                    lcd_set_cursor(1, 0);
                    lcd_string(temp_str);
                }
                else if (menu >= 0 && menu < menus.size())
                {

                    lcd_clear();

                    for (int line = 0; line < MAX_LINES; line++)
                    {
                        lcd_set_cursor(line, 0);
                        if (menu_item + line >= (uint)menus.at(menu).size())
                        {
                            lcd_string(const_cast<const char *>(std::string(16, ' ').c_str()));
                        }
                        else
                        {

                            const char *str = menus[menu][menu_item + line];
                            uint whitespace = 16 - strlen(str);
                            char *text = (char *)malloc(strlen(str) + whitespace);
                            strcpy(text, menus[menu][menu_item + line]);
                            if (menu_item + line == menu_item)
                            {
                                strcat(text, const_cast<const char *>((std::string(whitespace - 1, ' ') + "<").c_str()));
                            }
                            lcd_string(text);
                            free(text);
                        }
                    }
                }
            }
            xSemaphoreGive(i2c_default_mutex);
        }
        else
        {
            snprintf(msg.body, 128, "LCD Task: Failed to obtain I2C mutex\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
        }
        vTaskDelay(1);
    }
}

void obt_task(void *pvParameters)
{
    Message_t msg;
    snprintf(msg.body, 128, "OBT Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    while (1)
    {
        if (xSemaphoreTake(i2c_default_mutex, 1000) == pdTRUE)
        {
            uint16_t t;
            uint8_t buf[2];
            reg_read(i2c_default, LM75_BASE_ADDRESS, LM75_REGISTER_TEMP, buf, 2);
            t = buf[0] << 8 | buf[1];
            board_temp = (float)t / 256.0f;
            xSemaphoreGive(i2c_default_mutex);
        }
        else
        {
            snprintf(msg.body, 128, "OBT Task: Failed to obtain I2C mutex\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
        }
        vTaskDelay(1500);
    }
}

void dht_task(void *pvParameters)
{
    DHT_Data_t *dht = (DHT_Data_t *)pvParameters;
    Message_t msg;
    gpio_init(dht->pin);
    bool dht_initialized = false;
    if (DHT22_Init(dht->pin) != 0)
    {
        dht_initialized = false;
        snprintf(msg.body, 128, "DHT Sensor %d Init Failed on Pin %d\n", dht->id, dht->pin);
        xQueueSend(usbQueue, (void *)&msg, 10);
    }
    else
    {
        dht_initialized = true;
        snprintf(msg.body, 128, "DHT Sensor %d Initialized on Pin %d\n", dht->id, dht->pin);
        xQueueSend(usbQueue, (void *)&msg, 10);
    }
    while (1)
    {
        if (dht_initialized)
        {
            xQueueSend(led1Queue, (void *)true, 10);
            dht_reading reading;
            if (xSemaphoreTake(i2c_default_mutex, 1000) == pdTRUE)
            {
                DHT22_Read_Data(dht->pin, &reading);
                // double kelvin = reading.temp_celsius + 273.15;
                // double fahrenheit = (reading.temp_celsius * 9 / 5) + 32;
                dht_readings.at(dht->id) = {dht->id, dht->pin, reading.humidity, reading.temp_celsius};
                xSemaphoreGive(i2c_default_mutex);
            }
            else
            {
                snprintf(msg.body, 128, "DHT Task %d: Failed to obtain I2C mutex\n", dht->id);
                xQueueSend(usbQueue, (void *)&msg, 10);
            }
            xQueueSend(lcdQueue, (void *)(menu == -1 ? true : false), 10);
            xQueueSend(led1Queue, (void *)false, 10);
        }
        else
        {
            snprintf(msg.body, 128, "DHT Sensor %d not initialized. Removing task.\n", dht->id);
            xQueueSend(usbQueue, (void *)&msg, 10);
            break;
        }
        vTaskDelay(1500);
    }
    vTaskDelete(NULL);
}

void encoder_task(void *pvParameters)
{
    pinMode(ENCODER_A_PIN, GPIO_IN);
    pinMode(ENCODER_B_PIN, GPIO_IN);
    pinMode(ENCODER_BUTTON_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ENCODER_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    Message_t msg;
    snprintf(msg.body, 128, "Encoder Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    while (1)
    {
        if (menu > -1)
        {
            enc_a = gpio_get(ENCODER_A_PIN);
            if (enc_a != enc_a_last)
            {
                enc_b = gpio_get(ENCODER_B_PIN);
                if (!enc_a && enc_b)
                {
                    if (m > 0)
                    {
                        --m;
                        if (menu_item > 0)
                        {
                            menu_item--;
                            xQueueSend(lcdQueue, (void *)true, 0);
                        }
                    }
                }
                else if (!enc_a && !enc_b)
                {
                    if (m < (uint)menus.at(menu).size() - 1)
                    {
                        ++m;
                        if (menu_item < (uint)menus.at(menu).size())
                        {
                            menu_item++;
                            xQueueSend(lcdQueue, (void *)true, 0);
                        }
                    }
                }
                if (m > (uint)menus.at(menu).size())
                {
                    m = 0;
                }
            }
            enc_a_last = enc_a;
        }
        vTaskDelay(1);
    }
}

void i2c_scan_task(void *pvParameters)
{

    Message_t msg;
    bool led = false;
    bool scan = false;

    int count = 0;
    snprintf(msg.body, 128, "I2C Test Loop Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    while (1)
    {
        led = !led;
        xQueueSend(led2Queue, &led, 10);
        led = !led;
        xQueueSend(led3Queue, &led, 10);
        led = !led;
        if (count == 10 && !scan)
        {
            snprintf(msg.body, 128, "\nI2C Bus 0 Scan\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            snprintf(msg.body, 128, "   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            for (int addr = 0; addr < (1 << 7); ++addr)
            {
                if (addr % 16 == 0)
                {
                    snprintf(msg.body, 128, "%02x ", addr);
                    xQueueSend(usbQueue, (void *)&msg, 10);
                }

                // Perform a 1-byte dummy read from the probe address. If a slave
                // acknowledges this address, the function returns the number of bytes
                // transferred. If the address byte is ignored, the function returns
                // -1.

                // Skip over any reserved addresses.
                int ret;
                uint8_t rxdata;
                if (reserved_addr(addr))
                    ret = PICO_ERROR_GENERIC;
                else
                    ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

                snprintf(msg.body, 128, ret < 0 ? "." : "@");
                xQueueSend(usbQueue, (void *)&msg, 10);
                snprintf(msg.body, 128, addr % 16 == 15 ? "\n" : "  ");
                xQueueSend(usbQueue, (void *)&msg, 10);
            }
            snprintf(msg.body, 128, "Done.\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            snprintf(msg.body, 128, "\nI2C Bus 1 Scan\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            snprintf(msg.body, 128, "   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            for (int addr = 0; addr < (1 << 7); ++addr)
            {
                if (addr % 16 == 0)
                {
                    snprintf(msg.body, 128, "%02x ", addr);
                    xQueueSend(usbQueue, (void *)&msg, 10);
                }

                // Perform a 1-byte dummy read from the probe address. If a slave
                // acknowledges this address, the function returns the number of bytes
                // transferred. If the address byte is ignored, the function returns
                // -1.

                // Skip over any reserved addresses.
                int ret;
                uint8_t rxdata;
                if (reserved_addr(addr))
                    ret = PICO_ERROR_GENERIC;
                else
                    ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);

                snprintf(msg.body, 128, ret < 0 ? "." : "@");
                xQueueSend(usbQueue, (void *)&msg, 10);
                snprintf(msg.body, 128, addr % 16 == 15 ? "\n" : "  ");
                xQueueSend(usbQueue, (void *)&msg, 10);
            }
            snprintf(msg.body, 128, "Done.\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
            scan = true;
        }

        if (count < 10)
            count++;

        vTaskDelay(1000);
        //                                                                                                                                                      lcd_clear();
    }
}

void setup()
{
    Serial.begin(115200);
    sleep_ms(3000);
    Serial.println("Dry Box Controller Starting...");

    i2c_init(i2c_default, 100 * 1000);
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(I2C_0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_0_SDA_PIN);
    gpio_pull_up(I2C_0_SCL_PIN);
    gpio_set_function(I2C_1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_1_SDA_PIN);
    gpio_pull_up(I2C_1_SCL_PIN);

    pinMode(EEPROM_WP_PIN, GPIO_OUT);

    usbQueue = xQueueCreate(MSG_QUEUE_LEN, sizeof(Message_t));
    lcdQueue = xQueueCreate(1, sizeof(bool));
    led1Queue = xQueueCreate(1, sizeof(bool));
    led2Queue = xQueueCreate(1, sizeof(bool));
    led3Queue = xQueueCreate(1, sizeof(bool));

    i2c_default_mutex = xSemaphoreCreateMutex();

    dht_readings[0] = th1;
    dht_readings[1] = th2;

    xTaskCreate(usb_task, "USB Task", 256, NULL, 1, &usbTaskHandle);
    xTaskCreate(obt_task, "OBT Task", 256, (void *)&i2c1_inst, 1, &obtTaskHandle);
    xTaskCreate(dht_task, "DHT Task 1", 1024, (void *)&th1, 1, &dht1TaskHandle);
    xTaskCreate(dht_task, "DHT Task 2", 1024, (void *)&th2, 1, &dht2TaskHandle);
    xTaskCreate(led_task, "LED Task", 256, NULL, 1, &ledTaskHandle);
    xTaskCreate(lcd_task, "LCD Task", 1024, NULL, 1, &lcdTaskHandle);
    xTaskCreate(encoder_task, "ENC Task", 256, NULL, 1, &encoderTaskHandle);
    xTaskCreate(button_task, "BTN Task", 256, NULL, 1, &btnTaskHandle);
    xTaskCreate(i2c_scan_task, "I2C Scan Task", 256, NULL, 1, &i2cScanTaskHandle);
}

void loop()
{
    // Not used when using FreeRTOS, but required by Arduino
    vTaskDelay(pdMS_TO_TICKS(1));
}
