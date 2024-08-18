#pragma once

#include <Arduino.h>

// Wi-Fi credentials
extern const char *ssid;
extern const char *password;

// NTP Server
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;

// Pin Definitions
extern const int NTC_PIN;
extern const int EVAP_SENSOR_PIN;
extern const int COMPRESSOR_RELAY_PIN;
extern const int DEFROST_RELAY_PIN;
extern const int FAN_RELAY_PIN;
extern const int DOOR_SENSOR_PIN;

// NTC parameters
extern const float SERIES_RESISTOR;
extern const int ADC_MAX;

// Alert thresholds
extern const float TEMP_HIGH_ALERT;
extern const float TEMP_LOW_ALERT;

// Data logging parameters
extern const char *DATA_FILE;
extern const unsigned long LOG_INTERVAL;

// Data history size
constexpr  int DATA_HISTORY_SIZE = 1440;

// Global variables
extern float currentTemperature;
extern float evaporatorTemperature;
extern bool isDefrosting;
extern unsigned long lastDefrostTime;
extern bool highTempAlert;
extern bool lowTempAlert;
extern bool useSimulatedTemperature;
extern float simulatedTemperature;

extern unsigned long startupTime;
extern bool energySavingMode;

// Add these declarations
extern unsigned long drainingStartTime;
extern bool isDraining;