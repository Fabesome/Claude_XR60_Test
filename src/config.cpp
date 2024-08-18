#include "config.h"

// Wi-Fi credentials
const char *ssid = "";
const char *password = "";

// NTP Server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;  // Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  // Replace with your daylight savings offset (seconds)


// Pin Definitions
const int NTC_PIN = 34;
const int EVAP_SENSOR_PIN = 35;
const int COMPRESSOR_RELAY_PIN = 16;
const int DEFROST_RELAY_PIN = 17;
const int FAN_RELAY_PIN = 18;
const int DOOR_SENSOR_PIN = 19;

// NTC parameters
const float SERIES_RESISTOR = 10000;
const int ADC_MAX = 4095;

// Alert thresholds
const float TEMP_HIGH_ALERT = 10.0;
const float TEMP_LOW_ALERT = -5.0;

// Data logging parameters
const char *DATA_FILE = "/temperature_log.csv";
const unsigned long LOG_INTERVAL = 5000;

// Global variables
float currentTemperature = 0.0;
float evaporatorTemperature = 0.0;
bool isDefrosting = false;
unsigned long lastDefrostTime = 0;
bool highTempAlert = false;
bool lowTempAlert = false;
bool useSimulatedTemperature = false;
float simulatedTemperature = 20.0;
<<<<<<< HEAD
unsigned long startupTime = 0;
bool energySavingMode = false;

// Add these definitions
unsigned long drainingStartTime = 0;
bool isDraining = false;
=======
>>>>>>> daadbed8af6a5b4ef83ad228df3e0f594acefaa5
