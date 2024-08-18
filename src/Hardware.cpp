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

// State tracking variables
bool isCompressorOn = false;
bool isFanOn = false;
bool isDefrostOn = false;

void setupHardware() {
  pinMode(COMPRESSOR_RELAY_PIN, OUTPUT);
  pinMode(DEFROST_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  digitalWrite(COMPRESSOR_RELAY_PIN, LOW);  // Ensure compressor is initially off
  digitalWrite(DEFROST_RELAY_PIN, LOW);  // Ensure defrost is initially off
  digitalWrite(FAN_RELAY_PIN, LOW);  // Ensure fan is initially off

  attachInterrupt(digitalPinToInterrupt(DOOR_SENSOR_PIN), handleDigitalInput, CHANGE);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }
}

void IRAM_ATTR handleDigitalInput() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200) {
        if (energySavingMode) {
            exitEnergySavingMode();
        } else {
            enterEnergySavingMode();
        }
    }
    lastInterruptTime = interruptTime;
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

bool canActivateOutputs() {
    return (millis() - startupTime) >= (settings.OdS * 1000);
}

void controlCompressor() {
    if (!canActivateOutputs() || isDefrosting || isDraining) {
        return;  // Don't activate compressor yet
    }

    float effectiveSetpoint = settings.SEt;
    if (energySavingMode) {
        effectiveSetpoint += settings.HES;
    }

    bool shouldCompressorBeOn = false;

    if (currentTemperature > effectiveSetpoint + settings.Hy) {
        shouldCompressorBeOn = true;
    } else if (currentTemperature < effectiveSetpoint) {
        shouldCompressorBeOn = false;
    } else {
        // If temperature is between setpoint and setpoint + hysteresis,
        // maintain the current state to prevent short cycling
        shouldCompressorBeOn = isCompressorOn;
    }

    // Only change the compressor state if it's different from the current state
    if (shouldCompressorBeOn != isCompressorOn) {
        isCompressorOn = shouldCompressorBeOn;
        digitalWrite(COMPRESSOR_RELAY_PIN, isCompressorOn ? HIGH : LOW);
        Serial.printf("Compressor turned %s\n", isCompressorOn ? "ON" : "OFF");
    }

    // For debugging purposes
    Serial.printf("Current temp: %.2f, Set point: %.2f, Hysteresis: %.2f, Compressor: %s", 
                  currentTemperature, effectiveSetpoint, settings.Hy,
                  isCompressorOn ? "ON" : "OFF");
}

void handleDefrost() {
    if (!canActivateOutputs()) {
        return;  // Don't activate defrost yet
    }

    unsigned long currentTime = millis();
    bool shouldDefrostBeOn = false;

    // Start defrost if it's time and we're not already defrosting or draining
    if (!isDefrostOn && !isDraining && (currentTime - lastDefrostTime) > (settings.IdF * 3600000)) {
        shouldDefrostBeOn = true;
    }

    // End defrost if maximum duration is reached or temperature is above dtE
    if (isDefrostOn &&
        ((currentTime - lastDefrostTime) > (settings.MdF * 60000) ||
         evaporatorTemperature > settings.dtE)) {
        shouldDefrostBeOn = false;
        isDraining = true;
        drainingStartTime = currentTime;
    }

    // Handle draining time
    if (isDraining && (currentTime - drainingStartTime) > (settings.Fdt * 60000)) {
        isDraining = false;
    }

    // Only change the defrost state if it's different from the current state
    if (shouldDefrostBeOn != isDefrostOn) {
        isDefrostOn = shouldDefrostBeOn;
        digitalWrite(DEFROST_RELAY_PIN, isDefrostOn ? HIGH : LOW);
        Serial.printf("Defrost turned %s\n", isDefrostOn ? "ON" : "OFF");
        if (isDefrostOn) {
            lastDefrostTime = currentTime;
        }
    }
}

void controlFan() {
    if (!canActivateOutputs() || isDefrosting || isDraining) {
        if (isFanOn) {
            isFanOn = false;
            digitalWrite(FAN_RELAY_PIN, LOW);
            Serial.println("Fan turned OFF due to defrost/drain/startup delay");
        }
        return;
    }

    bool shouldFanBeOn = false;
    // Check if evaporator temperature is below FSt
    if (evaporatorTemperature <= settings.FSt || settings.P2P == "n") {
        // Check fan operating mode
        if (settings.FnC == "C_n" || settings.FnC == "C_Y") {
            // Fan runs with compressor
            shouldFanBeOn = isCompressorOn;
        } else if (settings.FnC == "O_n" || settings.FnC == "O_Y") {
            // Fan runs continuously
            shouldFanBeOn = true;
        }

        // Check temperature differential (Fct)
        if (settings.Fct > 0 && (currentTemperature - evaporatorTemperature) > settings.Fct) {
            shouldFanBeOn = true;
        }
    }

    // Only change the fan state if it's different from the current state
    if (shouldFanBeOn != isFanOn) {
        isFanOn = shouldFanBeOn;
        digitalWrite(FAN_RELAY_PIN, isFanOn ? HIGH : LOW);
        Serial.printf("Fan turned %s\n", isFanOn ? "ON" : "OFF");
    }


    Serial.printf(", Fan: %s\n", isFanOn ? "ON" : "OFF");
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
  if (isCompressorOn && (millis() - lastCompressorStartTime) < (settings.AC * 60000)) {
    Serial.println("Warning: Compressor short cycling detected");
  }
  if (isCompressorOn) {
    lastCompressorStartTime = millis();
  }

  // Check if defrost cycle is too long
  if (isDefrostOn && (millis() - lastDefrostTime) > (settings.MdF * 60000 * 1.1)) {
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
    float highAlarmThreshold, lowAlarmThreshold;

    if (settings.ALC == "rel") {
        // Relative alarm thresholds
        highAlarmThreshold = settings.SEt + settings.ALU;
        lowAlarmThreshold = settings.SEt + settings.ALL;  // ALL is typically negative
    } else {
        // Absolute alarm thresholds
        highAlarmThreshold = settings.ALU;
        lowAlarmThreshold = settings.ALL;
    }

    // Check high temperature alarm
    if (temperature > highAlarmThreshold) {
        if (!highTempAlert) {
            highTempAlert = true;
            Serial.printf("High temperature alarm activated. Temp: %.1f, Threshold: %.1f\n", temperature, highAlarmThreshold);
        }
    } else if (temperature < (highAlarmThreshold - settings.AFH)) {
        if (highTempAlert) {
            highTempAlert = false;
            Serial.printf("High temperature alarm deactivated. Temp: %.1f, Threshold: %.1f\n", temperature, highAlarmThreshold - settings.AFH);
        }
    }

    // Check low temperature alarm
    if (temperature < lowAlarmThreshold) {
        if (!lowTempAlert) {
            lowTempAlert = true;
            Serial.printf("Low temperature alarm activated. Temp: %.1f, Threshold: %.1f\n", temperature, lowAlarmThreshold);
        }
    } else if (temperature > (lowAlarmThreshold + settings.AFH)) {
        if (lowTempAlert) {
            lowTempAlert = false;
            Serial.printf("Low temperature alarm deactivated. Temp: %.1f, Threshold: %.1f\n", temperature, lowAlarmThreshold + settings.AFH);
        }
    }
}

void enterEnergySavingMode() {
    if (!energySavingMode) {
        energySavingMode = true;
        Serial.println("Entering Energy Saving Mode");
    }
}

void exitEnergySavingMode() {
    if (energySavingMode) {
        energySavingMode = false;
        Serial.println("Exiting Energy Saving Mode");
    }
}