#include <Arduino.h>
#include "config.h"
#include "Settings.h"
#include "Hardware.h"
#include "WebServer.h"
#include "DataLogger.h"
#include <SPIFFS.h>
#include <Time.h>



void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  loadSettings();
  setupHardware();
  setupWiFi(); //including NTP
  setupWebServer();
  setupDataLogging();

  startupTime = millis();

  Serial.println("Setup complete");
}

void loop()
{
  currentTemperature = readTemperature(false);
  Serial.printf("Current temperature: %.2f\n", currentTemperature);

  if (settings.P2P == "y")
  {
    evaporatorTemperature = readTemperature(true);
    Serial.printf("Evaporator temperature: %.2f\n", evaporatorTemperature);
  }

  checkErrors();
  checkAlerts(currentTemperature);

  if(!canActivateOutputs())
    Serial.println("Startup-Delay active!");

  controlCompressor();
  handleDefrost();
  controlFan();

  logDataIfNeeded();

  delay(3000); // Adjust as needed
}

