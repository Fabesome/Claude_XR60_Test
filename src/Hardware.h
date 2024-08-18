#pragma once

#include <Arduino.h>

extern unsigned long drainingStartTime;
extern bool isDraining;
extern bool isDefrosting;
extern bool isCompressorOn;
extern bool isFanOn;
extern bool isDefrostOn;


void setupHardware();
void setupWiFi();
float readTemperature(bool isEvaporatorSensor);
void calibrateSensor(float temp1, float temp2, float temp3);
void controlCompressor();
void handleDefrost();
void controlFan();
void checkErrors();
void checkAlerts(float temperature);
bool canActivateOutputs();
void enterEnergySavingMode();
void exitEnergySavingMode();
void IRAM_ATTR handleDigitalInput();

