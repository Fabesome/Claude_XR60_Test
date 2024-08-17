#include "DataLogger.h"
#include "config.h"
#include "Settings.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

struct DataPoint {
    unsigned long timestamp;
    float temperature;
    float evaporatorTemperature;
    bool compressorState;
    bool defrostState;
    bool fanState;
    bool doorOpen;
};

DataPoint dataHistory[DATA_HISTORY_SIZE];
int dataHistoryIndex = 0;
unsigned long lastLogTime = 0;

void setupDataLogging() {
    if (!SPIFFS.exists(DATA_FILE)) {
        File file = SPIFFS.open(DATA_FILE, FILE_WRITE);
        if (file) {
            file.println("Timestamp,Temperature,EvaporatorTemperature,CompressorState,DefrostState,FanState,DoorState");
            file.close();
        }
    }
}

void logDataIfNeeded() {
    if (millis() - lastLogTime >= LOG_INTERVAL) {
        bool doorOpen = (digitalRead(DOOR_SENSOR_PIN) == LOW);
        DataPoint newData = {
            millis(),
            currentTemperature,
            evaporatorTemperature,
            digitalRead(COMPRESSOR_RELAY_PIN) == HIGH,
            digitalRead(DEFROST_RELAY_PIN) == HIGH,
            digitalRead(FAN_RELAY_PIN) == HIGH,
            doorOpen
        };
        
        dataHistory[dataHistoryIndex] = newData;
        dataHistoryIndex = (dataHistoryIndex + 1) % DATA_HISTORY_SIZE;

        File file = SPIFFS.open(DATA_FILE, FILE_APPEND);
        if (file) {
            file.printf("%lu,%.2f,%.2f,%d,%d,%d,%d\n",
                        newData.timestamp,
                        newData.temperature,
                        settings.P2P == "y" ? newData.evaporatorTemperature : 0.0,
                        newData.compressorState,
                        newData.defrostState,
                        newData.fanState,
                        newData.doorOpen);
            file.close();
        }

        lastLogTime = millis();
    }
}

String getLatestDataJSON() {
    JsonDocument doc;
    JsonObject data = doc["data"].to<JsonObject>();
    int index = (dataHistoryIndex - 1 + DATA_HISTORY_SIZE) % DATA_HISTORY_SIZE;
    
    data["time"] = dataHistory[index].timestamp;
    data["temp"] = dataHistory[index].temperature;
    if (settings.P2P == "y") {
        data["evap_temp"] = dataHistory[index].evaporatorTemperature;
    }
    data["compressor"] = dataHistory[index].compressorState;
    data["defrost"] = dataHistory[index].defrostState;
    data["fan"] = dataHistory[index].fanState;
    data["door"] = dataHistory[index].doorOpen;

    String response;
    serializeJson(doc, response);
    return response;
}

String getDataJSON(unsigned long startTime, unsigned long endTime) {
    JsonDocument doc;
    JsonArray dataArray = doc["data"].to<JsonArray>();

    for (int i = 0; i < DATA_HISTORY_SIZE; i++) {
        int index = (dataHistoryIndex - 1 - i + DATA_HISTORY_SIZE) % DATA_HISTORY_SIZE;
        if (dataHistory[index].timestamp >= startTime && dataHistory[index].timestamp <= endTime) {
            JsonObject point = dataArray.add<JsonObject>();
            point["time"] = dataHistory[index].timestamp;
            point["temp"] = dataHistory[index].temperature;
            if (settings.P2P == "y") {
                point["evap_temp"] = dataHistory[index].evaporatorTemperature;
            }
            point["compressor"] = dataHistory[index].compressorState;
            point["defrost"] = dataHistory[index].defrostState;
            point["fan"] = dataHistory[index].fanState;
            point["door"] = dataHistory[index].doorOpen;
        }
    }

    String response;
    serializeJson(doc, response);
    return response;
}