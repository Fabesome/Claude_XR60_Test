#pragma once

#include <Arduino.h>

void setupDataLogging();
void logDataIfNeeded();
String getLatestDataJSON();
String getDataJSON(unsigned long startTime, unsigned long endTime);