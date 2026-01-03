#include "main.h"

bool adjusting_setting = false;

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

uint8_t volume = 100; // Speaker volume 0-100
uint8_t tmin = 20;    // Minimum temperature in Celsius
uint8_t tmax = 40;    // Maximum temperature in Celsius
uint8_t ttarget = 21; // Target temperature in Celsius
uint8_t unit = 0;     // 0 = Celsius, 1 = Fahrenheit
uint8_t hmin = 0;     // Minimum humidity in %
uint8_t hmax = 15;    // Maximum humidity in %
uint8_t htarget = 7;  // Target humidity in %

std::string replace_placeholders(const char *src)
{
    std::string s(src);
    struct Placeholder
    {
        const char *key;
        std::string value;
    };

    int tmin_disp = tmin;
    int tmax_disp = tmax;
    int ttarget_disp = ttarget;
    std::string unit_str = "C";
    if (unit == 1)
    { // Fahrenheit
        tmin_disp = static_cast<int>(round(tmin * 9.0 / 5.0 + 32));
        tmax_disp = static_cast<int>(round(tmax * 9.0 / 5.0 + 32));
        ttarget_disp = static_cast<int>(round(ttarget * 9.0 / 5.0 + 32));
        unit_str = "F";
    }
    std::vector<Placeholder> replacements = {
        {"%x7f%", std::string(1, '\x7f')},
        {"%volume%", std::to_string(volume)},
        {"%tmin%", std::to_string(tmin_disp)},
        {"%tmax%", std::to_string(tmax_disp)},
        {"%ttarget%", std::to_string(ttarget_disp)},
        {"%unit%", unit_str},
        {"%hmin%", std::to_string(hmin)},
        {"%hmax%", std::to_string(hmax)},
        {"%htarget%", std::to_string(htarget)}};

    for (const auto &repl : replacements)
    {
        size_t pos;
        while ((pos = s.find(repl.key)) != std::string::npos)
        {
            s.replace(pos, strlen(repl.key), repl.value);
        }
    }
    return s;
}

void save_settings()
{
    eeprom_write_byte(0, volume);
    eeprom_write_byte(1, tmin);
    eeprom_write_byte(2, tmax);
    eeprom_write_byte(3, ttarget);
    eeprom_write_byte(4, unit);
    eeprom_write_byte(5, hmin);
    eeprom_write_byte(6, hmax);
    eeprom_write_byte(7, htarget);
}

void process_result(byte &result, const char *setting_name)
{
    Message_t msg;
    switch (result)
    {
    case EEPROM_ADDR_ERR:
        snprintf(msg.body, sizeof(msg.body), "EEPROM address error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 1:
        snprintf(msg.body, sizeof(msg.body), "Data too long error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 2:
        snprintf(msg.body, sizeof(msg.body), "NACK on transmit of address error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 3:
        snprintf(msg.body, sizeof(msg.body), "NACK on transmit of data error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 4:
        snprintf(msg.body, sizeof(msg.body), "Other error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 5:
        snprintf(msg.body, sizeof(msg.body), "Timeout error when saving %s setting\n", setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    case 0:
        break;
    default:
        snprintf(msg.body, sizeof(msg.body), "Unknown I2C error (%d) when saving %s setting\n", result, setting_name);
        msg.level = LOG_ERROR;
        xQueueSend(usbQueue, (void *)&msg, 0);
        break;
    }
    result = 0;
}

void load_settings()
{
    Message_t msg;
    byte result = 0;
    volume = eeprom_read_byte(0);
    if (volume > 100)
    {
        volume = 100;
        result = save_setting(0, volume);
        process_result(result, "volume");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded volume setting: %d%%\n", volume);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    tmin = eeprom_read_byte(1);
    if (tmin > 100)
    {
        tmin = 100;
        result = save_setting(1, tmin);
        process_result(result, "tmin");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded tmin setting: %dC\n", tmin);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    tmax = eeprom_read_byte(2);
    if (tmax > 100)
    {
        tmax = 100;
        result = save_setting(2, tmax);
        process_result(result, "tmax");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded tmax setting: %dC\n", tmax);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    ttarget = eeprom_read_byte(3);
    if (ttarget > 100)
    {
        ttarget = 100;
        result = save_setting(3, ttarget);
        process_result(result, "ttarget");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded ttarget setting: %dC\n", ttarget);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    unit = eeprom_read_byte(4);
    if (unit > 1)
    {
        unit = 0;
        result = save_setting(4, unit);
        process_result(result, "unit");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded unit setting: %s\n", unit == 0 ? "Celsius" : "Fahrenheit");
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    hmin = eeprom_read_byte(5);
    if (hmin > 100)
    {
        hmin = 100;
        result = save_setting(5, hmin);
        process_result(result, "hmin");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded hmin setting: %d%%\n", hmin);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    hmax = eeprom_read_byte(6);
    if (hmax > 100)
    {
        hmax = 100;
        result = save_setting(6, hmax);
        process_result(result, "hmax");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded hmax setting: %d%%\n", hmax);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
    htarget = eeprom_read_byte(7);
    if (htarget > 100)
    {
        htarget = 100;
        result = save_setting(7, htarget);
        process_result(result, "htarget");
    }
    else
    {
        snprintf(msg.body, sizeof(msg.body), "Loaded htarget setting: %d%%\n", htarget);
        msg.level = LOG_INFO;
        xQueueSend(usbQueue, (void *)&msg, 0);
    }
}

void gpio_callback(uint gpio, uint32_t events)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_button_press < 200)
    {
        return;
    }
    last_button_press = now;

    switch (gpio)
    {
    case 14:
        button_event = true;
        break;
    default:
        break;
    }
}

void button_task(void *pvParameters)
{
    TwoWire myWire = TwoWire(i2c1, I2C_1_SDA_PIN, I2C_1_SCL_PIN);
    myWire.begin();
    eeprom.begin(eeprom.twiClock100kHz, &myWire);
    digitalWrite(EEPROM_WP_PIN, LOW);
    load_settings();
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
                case 1:
                    // DHT Sensor 1 settings
                    break;
                case 2:
                    // DHT Sensor 2 settings
                    break;
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
                case 1:
                    // Thermal 1 settings
                    break;
                case 2:
                    // Thermal 2 settings
                    break;
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
                case 1:
                    // Output 1 settings
                    break;
                case 2:
                    // Output 2 settings
                    break;
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
                case 1:
                    adjusting_setting = !adjusting_setting;
                    if (!adjusting_setting)
                    {
                        byte result = save_setting(0, volume);
                        process_result(result, "volume");
                        menu_item = 0;
                        m = 0;
                    }
                    break;
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
            case HMIN_MENU:
            case HMAX_MENU:
            case HTARGET_MENU:
                switch (menu_item)
                {
                case 1:
                    adjusting_setting = !adjusting_setting;
                    if (!adjusting_setting)
                    {
                        if (menu == HMIN_MENU)
                        {
                            save_setting(5, hmin);
                        }
                        else if (menu == HMAX_MENU)
                        {
                            save_setting(6, hmax);
                        }
                        else if (menu == HTARGET_MENU)
                        {
                            save_setting(7, htarget);
                        }
                        menu_item = 0;
                        m = 0;
                    }
                    break;
                default:
                    menu = HUMIDITY_MENU;
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
            case TMIN_MENU:
            case TMAX_MENU:
            case TTARGET_MENU:
                switch (menu_item)
                {
                case 1:
                    adjusting_setting = !adjusting_setting;
                    if (!adjusting_setting)
                    {
                        uint8_t tmin_to_save = tmin;
                        uint8_t tmax_to_save = tmax;
                        uint8_t ttarget_to_save = ttarget;
                        if (unit == 1)
                        {
                            tmin_to_save = round((tmin - 32) * 5.0 / 9.0);
                            tmax_to_save = round((tmax - 32) * 5.0 / 9.0);
                            ttarget_to_save = round((ttarget - 32) * 5.0 / 9.0);
                        }
                        if (menu == TMIN_MENU)
                        {
                            save_setting(1, tmin_to_save);
                        }
                        else if (menu == TMAX_MENU)
                        {
                            save_setting(2, tmax_to_save);
                        }
                        else if (menu == TTARGET_MENU)
                        {
                            save_setting(3, ttarget_to_save);
                        }
                        menu_item = 0;
                        m = 0;
                    }
                    break;
                default:
                    menu = TEMPERATURE_MENU;
                    menu_item = 0;
                    m = 0;
                    break;
                }
                break;
            case UNIT_MENU:
                switch (menu_item)
                {
                case 1:
                    unit = (unit + 1) % 2;
                    menu_item = 0;
                    m = 0;
                    save_setting(4, unit);
                    break;
                default:
                    menu = TEMPERATURE_MENU;
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
        vTaskDelay(pdMS_TO_TICKS(1));
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
    msg.level = LOG_DEBUG;
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
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void usb_task(void *pvParameters)
{

    DEBUG_PRINTLN("USB Task Started");
    Message_t rcv_msg;
    while (1)
    {
        xQueueReceive(usbQueue, (void *)&rcv_msg, portMAX_DELAY);

        switch (rcv_msg.level)
        {
        case LOG_NONE:
            break;
        case LOG_DEBUG:
            DEBUG_PRINTF("%s", rcv_msg.body);
            break;
        case LOG_INFO:
            Serial.printf("%s", rcv_msg.body);
            break;
        case LOG_WARN:
            Serial.printf("%s", rcv_msg.body);
            break;
        case LOG_ERROR:
            Serial.printf("%s", rcv_msg.body);
            break;
        default:
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void lcd_task(void *pvParameters)
{

    Message_t msg;
    msg.level = LOG_DEBUG;
    snprintf(msg.body, 128, "LCD Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    lcd_init();
    bool update = false;
    if (xQueueSend(lcdQueue, (void *)true, 10) != pdPASS)
    {
        msg.level = LOG_ERROR;
        snprintf(msg.body, 128, "LCD Task: Failed to send initial update message\n");
        xQueueSend(usbQueue, (void *)&msg, 10);
    }
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
                    char unit_str[2] = "C";
                    if (unit == 1)
                    {
                        temp = temp * 9.0 / 5.0 + 32.0;
                        unit_str[0] = 'F';
                    }
                    char temp_str[17];
                    snprintf(temp_str, 17, "%6.1f%s %-5.1f%% ", temp, unit_str, hum);
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
                            std::string replaced_str = replace_placeholders(str);

                            uint whitespace = 16 - replaced_str.length();
                            char *text = (char *)malloc(replaced_str.length() + whitespace);
                            strcpy(text, replaced_str.c_str());
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
            msg.level = LOG_ERROR;
            snprintf(msg.body, 128, "LCD Task: Failed to obtain I2C mutex\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void obt_task(void *pvParameters)
{
    Message_t msg;
    msg.level = LOG_DEBUG;
    snprintf(msg.body, 128, "OBT Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    // LM75 sensor = LM75(LM75_BASE_ADDRESS);
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
            msg.level = LOG_ERROR;
            snprintf(msg.body, 128, "OBT Task: Failed to obtain I2C mutex\n");
            xQueueSend(usbQueue, (void *)&msg, 10);
        }

        vTaskDelay(pdMS_TO_TICKS(1500));
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
        msg.level = LOG_ERROR;
        snprintf(msg.body, 128, "DHT Sensor %d Init Failed on Pin %d\n", dht->id, dht->pin);
        xQueueSend(usbQueue, (void *)&msg, 10);
    }
    else
    {
        dht_initialized = true;
        msg.level = LOG_DEBUG;
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
                msg.level = LOG_ERROR;
                snprintf(msg.body, 128, "DHT Task %d: Failed to obtain I2C mutex\n", dht->id);
                xQueueSend(usbQueue, (void *)&msg, 10);
            }
            xQueueSend(lcdQueue, (void *)(menu == -1 ? true : false), 10);
            xQueueSend(led1Queue, (void *)false, 10);
        }
        else
        {
            msg.level = LOG_WARN;
            snprintf(msg.body, 128, "DHT Sensor %d not initialized. Removing task.\n", dht->id);
            xQueueSend(usbQueue, (void *)&msg, 10);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1500));
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
    msg.level = LOG_DEBUG;
    snprintf(msg.body, 128, "Encoder Task Started\n");
    xQueueSend(usbQueue, (void *)&msg, 10);
    while (1)
    {
        if (menu > -1 && menu < menus.size() && adjusting_setting == false)
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
        else
        {
            // In setting adjustment mode?
            if (adjusting_setting)
            {
                enc_a = gpio_get(ENCODER_A_PIN);
                if (enc_a != enc_a_last)
                {
                    enc_b = gpio_get(ENCODER_B_PIN);
                    if (!enc_a && enc_b)
                    {
                        // Clockwise
                        switch (menu)
                        {
                        case SPEAKER_MENU:
                            if (volume > 0)
                                volume -= 1;
                            else
                                volume = 0;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TMIN_MENU:
                            if (tmin > 0)
                                tmin -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TMAX_MENU:
                            if (tmax > 0)
                                tmax -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TTARGET_MENU:
                            if (ttarget > 0)
                                ttarget -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HMIN_MENU:
                            if (hmin > 0)
                                hmin -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HMAX_MENU:
                            if (hmax > 0)
                                hmax -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HTARGET_MENU:
                            if (htarget > 0)
                                htarget -= 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        default:
                            break;
                        }
                    }
                    else if (!enc_a && !enc_b)
                    {
                        // Counter-clockwise
                        switch (menu)
                        {
                        case SPEAKER_MENU:
                            if (volume < 100)
                                volume += 1;
                            else
                                volume = 100;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TMIN_MENU:
                            if (tmin < 100)
                                tmin += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TMAX_MENU:
                            if (tmax < 100)
                                tmax += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case TTARGET_MENU:
                            if (ttarget < 100)
                                ttarget += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HMIN_MENU:
                            if (hmin < 100)
                                hmin += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HMAX_MENU:
                            if (hmax < 100)
                                hmax += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        case HTARGET_MENU:
                            if (htarget < 100)
                                htarget += 1;
                            xQueueSend(lcdQueue, (void *)true, 0);
                            break;
                        default:
                            break;
                        }
                    }
                    enc_a_last = enc_a;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void i2c_scan_task(void *pvParameters)
{

    Message_t msg;
    bool led = false;
    bool scan = false;

    int count = 0;
    msg.level = LOG_DEBUG;
    snprintf(msg.body, 128, "I2C Scan Started\n");
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
            msg.level = LOG_INFO;
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
            vTaskDelete(i2cScanTaskHandle);
        }

        if (count < 10)
            count++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup()
{
    Serial.begin(115200);
    sleep_ms(3000);
    Serial.println("Dry Box Controller Starting...");

    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(I2C_0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_0_SDA_PIN);
    gpio_pull_up(I2C_0_SCL_PIN);

    pinMode(EEPROM_WP_PIN, GPIO_OUT);

    usbQueue = xQueueCreate(MSG_QUEUE_LEN, sizeof(Message_t));
    lcdQueue = xQueueCreate(4, sizeof(bool));
    led1Queue = xQueueCreate(4, sizeof(bool));
    led2Queue = xQueueCreate(4, sizeof(bool));
    led3Queue = xQueueCreate(4, sizeof(bool));

    i2c_default_mutex = xSemaphoreCreateMutex();
    i2c1_mutex = xSemaphoreCreateMutex();

    dht_readings[0] = th1;
    dht_readings[1] = th2;

    xTaskCreate(usb_task, "USB Task", 1024, NULL, 1, &usbTaskHandle);
    xTaskCreate(led_task, "LED Task", 1024, NULL, 1, &ledTaskHandle);
    xTaskCreate(encoder_task, "ENC Task", 1024, NULL, 1, &encoderTaskHandle);
    xTaskCreate(button_task, "BTN Task", 1024, NULL, 1, &btnTaskHandle);
#ifdef I2C_SCAN
    xTaskCreate(i2c_scan_task, "I2C Scan Task", 1024, NULL, 1, &i2cScanTaskHandle);
#else
    xTaskCreate(lcd_task, "LCD Task", 1024, NULL, 1, &lcdTaskHandle);
    xTaskCreate(obt_task, "OBT Task", 1024, (void *)&i2c0_inst, 1, &obtTaskHandle);
    xTaskCreate(dht_task, "DHT Task 1", 1024, (void *)&th1, 1, &dht1TaskHandle);
    xTaskCreate(dht_task, "DHT Task 2", 1024, (void *)&th2, 1, &dht2TaskHandle);

#endif
}

void loop()
{
    // Not used when using FreeRTOS, but required by Arduino
    vTaskDelay(pdMS_TO_TICKS(1));
}
