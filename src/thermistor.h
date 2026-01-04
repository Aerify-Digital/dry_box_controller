#ifndef THERMISTOR_H
#define THERMISTOR_H
#include <Arduino.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

typedef struct
{
    double temp_celsius;
} thermistor_reading;

typedef struct
{
    uint id;
    uint adc_input;
    uint pin;
    double celsius;
} Thermistor_Data_t;

// Read the ADC value (12 bits, 0-4095)
static inline uint16_t thermistor_adc_read(uint adc_input)
{
    adc_select_input(adc_input);
    return adc_read();
}

#endif