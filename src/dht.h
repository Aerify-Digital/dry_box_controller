#ifndef DHT_H
#define DHT_H
#include <Arduino.h>
#include "pico/stdlib.h"

typedef struct
{
    double humidity;
    double temp_celsius;
} dht_reading;

typedef struct
{
    uint id;
    uint pin;
    double humidity;
    double celsius;
} DHT_Data_t;

void DHT22_Rst(uint pin)
{
    gpio_set_dir(pin, GPIO_OUT); // SET OUTPUT
    gpio_put(pin, 0);            // GPIOA.0=0
    sleep_ms(20);                // Pull down Least 18ms
    gpio_put(pin, 1);            // GPIOA.0=1
    sleep_us(30);                // Pull up 20~40us
}

uint DHT22_Check(uint pin)
{
    uint retry = 0;
    gpio_set_dir(pin, GPIO_IN);          // SET INPUT
    while (gpio_get(pin) && retry < 100) // DHT22 Pull down 40~80us
    {
        retry++;
        sleep_us(1);
    };
    if (retry >= 100)
        return 1;
    else
        retry = 0;
    while (!gpio_get(pin) && retry < 100) // DHT22 Pull up 40~80us
    {
        retry++;
        sleep_us(1);
    };
    if (retry >= 100)
        return 1; // chack error
    return 0;
}

uint DHT22_Read_Bit(uint pin)
{
    uint retry = 0;
    while (gpio_get(pin) && retry < 100) // wait become Low level
    {
        retry++;
        sleep_us(1);
    }
    retry = 0;
    while (!gpio_get(pin) && retry < 100) // wait become High level
    {
        retry++;
        sleep_us(1);
    }
    sleep_us(40); // wait 40us
    if (gpio_get(pin))
        return 1;
    else
        return 0;
}

uint DHT22_Read_Byte(uint pin)
{
    uint i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT22_Read_Bit(pin);
    }
    return dat;
}

uint DHT22_Read_Data(uint pin, dht_reading *result)
{
    uint buf[5];
    uint i;
    DHT22_Rst(pin);
    if (DHT22_Check(pin) == 0)
    {
        for (i = 0; i < 5; i++)
        {
            buf[i] = DHT22_Read_Byte(pin);
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            result->humidity = ((buf[0] << 8) + buf[1]) / 10;
            result->temp_celsius = ((buf[2] << 8) + buf[3]) / 10;
        }
    }
    else
        return 1;
    return 0;
}
uint DHT22_Init(uint pin)
{
    DHT22_Rst(pin);
    return DHT22_Check(pin);
}
#endif