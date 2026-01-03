#ifndef MENU_H
#define MENU_H
#include <Arduino.h>
#include <vector>

std::vector<const char *> main_menu =
    {
        "%x7f%Info Screen", "Hardware",
        "Humidity", "Temperature"};

std::vector<const char *> hardware_menu =
    {
        "%x7f%Settings", "DHT Sensors",
        "Thermal Sensors", "Output Settings", "Speaker Volume"};

std::vector<const char *> dht_menu =
    {
        "%x7f%Hardware", "DHT 1", "DHT 2"};

std::vector<const char *> dht1_menu =
    {
        "%x7f%DHT Sensors", "%dht1_info%"};

std::vector<const char *> thermal_menu =
    {
        "%x7f%Hardware", "Thermistor 1", "Thermistor 2"};

std::vector<const char *> output_menu =
    {
        "%x7f%Hardware", "Output 1", "Output 2"};

std::vector<const char *> speaker_menu =
    {
        "%x7f%Hardware", "Volume %volume%%"};

std::vector<const char *> humidity_menu =
    {
        "%x7f%Settings", "Minimum", "Maximum",
        "Target"};

std::vector<const char *> hmin_menu =
    {
        "%x7f%Humidity", "Min %hmin%%"};

std::vector<const char *> hmax_menu =
    {
        "%x7f%Humidity", "Max %hmax%%"};
std::vector<const char *> htarget_menu =
    {
        "%x7f%Humidity", "Target %htarget%%"};

std::vector<const char *> temperature_menu =
    {
        "%x7f%Settings", "Minimum", "Maximum",
        "Target", "Unit"};

std::vector<const char *> tmin_menu =
    {
        "%x7f%Temperature", "Min %tmin%%unit%"};

std::vector<const char *> tmax_menu =
    {
        "%x7f%Temperature", "Max %tmax%%unit%"};
std::vector<const char *> ttarget_menu =
    {
        "%x7f%Temperature", "Target %ttarget%%unit%"};

std::vector<const char *> unit_menu =
    {
        "%x7f%Temperature", "%unit%"};

std::vector<std::vector<const char *>> menus = {main_menu, hardware_menu, humidity_menu, temperature_menu, unit_menu, dht_menu, thermal_menu, output_menu, speaker_menu, hmin_menu, hmax_menu, htarget_menu, tmin_menu, tmax_menu, ttarget_menu};

const uint HARDWARE_BTN = 1;
const uint DHT_BTN = 1;
const uint THERMAL_BTN = 2;
const uint OUTPUT_BTN = 3;
const uint SPEAKER_BTN = 4;

const uint HUMIDITY_BTN = 2;

const uint TEMPERATURE_BTN = 3;

const int MAIN_MENU = 0;
const int HARDWARE_MENU = 1;
const int DHT_MENU = 5;
const int THERMAL_MENU = 6;
const int OUTPUT_MENU = 7;
const int SPEAKER_MENU = 8;
const int HUMIDITY_MENU = 2;
const int HMIN_MENU = 9;
const int HMAX_MENU = 10;
const int HTARGET_MENU = 11;
const int TEMPERATURE_MENU = 3;
const int TMIN_MENU = 12;
const int TMAX_MENU = 13;
const int TTARGET_MENU = 14;
const int UNIT_MENU = 4;

#endif