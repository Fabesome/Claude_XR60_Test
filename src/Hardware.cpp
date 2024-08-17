#include "Hardware.h"
#include "config.h"
#include "Settings.h"
#include <Adafruit_BME280.h>
#include <WiFi.h>

Adafruit_BME280 bme;

// Steinhart-Hart coefficients
float A = 0.001129148;
float B = 0.000234125;
float C = 0.0000000876741;

void setupHardware() {
  pinMode(COMPRESSOR_RELAY_PIN, OUTPUT);
  pinMode(DEFROST_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }
}

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

float readTemperature(bool isEvaporatorSensor) {
  if (useSimulatedTemperature) {
    return simulatedTemperature;
  }
    
  float temperature;
  int sensorPin = isEvaporatorSensor ? EVAP_SENSOR_PIN : NTC_PIN;
    
  if (settings.useBME280 && !isEvaporatorSensor) {
    temperature = bme.readTemperature();
  } else {
    int adcValue = analogRead(sensorPin);
    float resistance = SERIES_RESISTOR / ((ADC_MAX / (float)adcValue) - 1);
    float logR = log(resistance);
    temperature = 1.0 / (A + B * logR + C * logR * logR * logR);
    temperature = temperature - 273.15;  // Convert Kelvin to Celsius
  }
    
  if (isEvaporatorSensor) {
    temperature += settings.OE;
  } else {
    temperature += settings.Ot;
  }
    
  return temperature;
}

void calibrateSensor(float temp1, float temp2, float temp3) {
  int adc1 = analogRead(NTC_PIN);
  delay(1000);
  int adc2 = analogRead(NTC_PIN);
  delay(1000);
  int adc3 = analogRead(NTC_PIN);

  float r1 = SERIES_RESISTOR / ((ADC_MAX / (float)adc1) - 1);
  float r2 = SERIES_RESISTOR / ((ADC_MAX / (float)adc2) - 1);
  float r3 = SERIES_RESISTOR / ((ADC_MAX / (float)adc3) - 1);

  float t1 = temp1 + 273.15;
  float t2 = temp2 + 273.15;
  float t3 = temp3 + 273.15;

  float L1 = log(r1);
  float L2 = log(r2);
  float L3 = log(r3);

  float Y1 = 1.0 / t1;
  float Y2 = 1.0 / t2;
  float Y3 = 1.0 / t3;

  float U2 = (Y2 - Y1) / (L2 - L1);
  float U3 = (Y3 - Y1) / (L3 - L1);

  C = (U3 - U2) / (L3 - L2) / (L1 + L2 + L3);
  B = U2 - C * (L1 * L1 + L1 * L2 + L2 * L2);
  A = Y1 - (B + L1 * L1 * C) * L1;
}

void controlCompressor() {
  Serial.printf("Controlling compressor: Current temp: %.2f, Set point: %.2f, Hysteresis: %.2f\n", 
                currentTemperature, settings.SEt, settings.Hy);
  if (currentTemperature > settings.SEt + settings.Hy) {
    digitalWrite(COMPRESSOR_RELAY_PIN, HIGH);  // Turn on compressor
    Serial.println("Compressor turned ON");
  } else if (currentTemperature < settings.SEt) {
    digitalWrite(COMPRESSOR_RELAY_PIN, LOW);   // Turn off compressor
    Serial.println("Compressor turned OFF");
  }
}

void handleDefrost() {
  unsigned long currentTime = millis();

  // Start defrost if it's time and we're not already defrosting
  if (!isDefrosting && (currentTime - lastDefrostTime) > (settings.IdF * 3600000)) {
    isDefrosting = true;
    digitalWrite(DEFROST_RELAY_PIN, HIGH);
    lastDefrostTime = currentTime;
  }

  // End defrost if maximum duration is reached or temperature is above dtE
  if (isDefrosting &&
      ((currentTime - lastDefrostTime) > (settings.MdF * 60000) ||
       currentTemperature > settings.dtE)) {
    isDefrosting = false;
    digitalWrite(DEFROST_RELAY_PIN, LOW);
  }
}

void controlFan() {
  if (settings.FnC == "C_n" || settings.FnC == "C_Y") {
    // Fan runs with compressor
    digitalWrite(FAN_RELAY_PIN, digitalRead(COMPRESSOR_RELAY_PIN));
  } else if (settings.FnC == "O_n" || settings.FnC == "O_Y") {
    // Fan runs continuously
    digitalWrite(FAN_RELAY_PIN, HIGH);
  }

  // Turn off fan during defrost if required
  if (isDefrosting && (settings.FnC == "C_n" || settings.FnC == "O_n")) {
    digitalWrite(FAN_RELAY_PIN, LOW);
  }
}

void checkErrors() {
  // Check if temperature is within expected range
  if (currentTemperature < -50 || currentTemperature > 150) {
    Serial.println("Error: Temperature out of expected range");
  }

  // Check if BME280 is selected but not responding
  if (settings.useBME280 && !bme.begin(0x76)) {
    Serial.println("Error: BME280 selected but not responding");
  }

  // Check if compressor is short cycling
  static unsigned long lastCompressorStartTime = 0;
  if (digitalRead(COMPRESSOR_RELAY_PIN) == HIGH && 
      (millis() - lastCompressorStartTime) < (settings.AC * 60000)) {
    Serial.println("Warning: Compressor short cycling detected");
  }
  if (digitalRead(COMPRESSOR_RELAY_PIN) == HIGH) {
    lastCompressorStartTime = millis();
  }

  // Check if defrost cycle is too long
  if (isDefrosting && (millis() - lastDefrostTime) > (settings.MdF * 60000 * 1.1)) {
    Serial.println("Warning: Defrost cycle exceeding maximum duration");
  }

  // Check if door has been open too long
  static unsigned long doorOpenTime = 0;
  if (digitalRead(DOOR_SENSOR_PIN) == LOW) {
    if (doorOpenTime == 0) {
      doorOpenTime = millis();
    } else if ((millis() - doorOpenTime) > 300000) { // 5 minutes
      Serial.println("Warning: Door has been open for more than 5 minutes");
    }
  } else {
    doorOpenTime = 0;
  }
}

void checkAlerts(float temperature) {
  if (temperature > TEMP_HIGH_ALERT) {
    highTempAlert = true;
  } else if (temperature < TEMP_LOW_ALERT) {
    lowTempAlert = true;
  } else {
    highTempAlert = false;
    lowTempAlert = false;
  }
}