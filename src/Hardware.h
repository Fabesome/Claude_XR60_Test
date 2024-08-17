#pragma once

#include <Arduino.h>

void setupHardware();
void setupWiFi();
float readTemperature(bool isEvaporatorSensor);
void calibrateSensor(float temp1, float temp2, float temp3);
void controlCompressor();
void handleDefrost();
void controlFan();
void checkErrors();
void checkAlerts(float temperature);