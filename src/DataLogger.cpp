#include "DataLogger.h"
#include "config.h"
#include "Settings.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Time.h>

struct DataPoint {
    time_t timestamp;
    float temperature;
    bool compressorState;
    bool defrostState;
    bool fanState;
    int remainingDefrostTime;
    int remainingDripTime;
};

DataPoint dataHistory[DATA_HISTORY_SIZE];
int dataHistoryIndex = 0;
unsigned long lastLogTime = 0;

void setupDataLogging() {
    if (!SPIFFS.exists(DATA_FILE)) {
        File file = SPIFFS.open(DATA_FILE, FILE_WRITE);
        if (file) {
            file.println("Date,Time,Temperature,CompressorState,DefrostState,FanState,RemainingDefrostTime,RemainingDripTime");
            file.close();
        }
    }
}

void logDataIfNeeded() {
    if (millis() - lastLogTime >= 5000) { // Log every 5 seconds
        time_t now = time(nullptr);
        
        // Use currentTemperature instead of calling readTemperature
        float temp = currentTemperature;
        
        DataPoint newData = {
            now,
            temp,
            digitalRead(COMPRESSOR_RELAY_PIN) == HIGH,
            digitalRead(DEFROST_RELAY_PIN) == HIGH,
            digitalRead(FAN_RELAY_PIN) == HIGH,
            isDefrosting ? (int)((settings.MdF * 60000 - (millis() - lastDefrostTime)) / 1000) : 0,
            isDraining ? (int)((settings.Fdt * 60000 - (millis() - drainingStartTime)) / 1000) : 0
        };
        
        dataHistory[dataHistoryIndex] = newData;
        dataHistoryIndex = (dataHistoryIndex + 1) % DATA_HISTORY_SIZE;

        File file = SPIFFS.open(DATA_FILE, FILE_APPEND);
        if (file) {
            char dateStr[11];
            char timeStr[9];
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&now));
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&now));
            
            file.printf("%s,%s,%.1f,%d,%d,%d,%d,%d\n",
                        dateStr,
                        timeStr,
                        newData.temperature,
                        newData.compressorState,
                        newData.defrostState,
                        newData.fanState,
                        newData.remainingDefrostTime,
                        newData.remainingDripTime);
            file.close();
        } else {
            Serial.println("Error: Failed to open log file");
        }

        lastLogTime = millis();
    }
}

String getLatestDataJSON() {
    JsonDocument doc;
    JsonObject data = doc.to<JsonObject>();
    int index = (dataHistoryIndex - 1 + DATA_HISTORY_SIZE) % DATA_HISTORY_SIZE;
    
    char dateStr[11];
    char timeStr[9];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&dataHistory[index].timestamp));
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&dataHistory[index].timestamp));
    
    data["date"] = dateStr;
    data["time"] = timeStr;
    data["temp"] = dataHistory[index].temperature;
    data["compressor"] = dataHistory[index].compressorState;
    data["defrost"] = dataHistory[index].defrostState;
    data["fan"] = dataHistory[index].fanState;
    data["remainingDefrostTime"] = dataHistory[index].remainingDefrostTime;
    data["remainingDripTime"] = dataHistory[index].remainingDripTime;

    String response;
    serializeJson(doc, response);
    return response;
}

String getDataJSON(unsigned long startTime, unsigned long endTime) {
    JsonDocument doc;
    JsonArray dataArray = doc.to<JsonArray>();

    for (int i = 0; i < DATA_HISTORY_SIZE; i++) {
        int index = (dataHistoryIndex - 1 - i + DATA_HISTORY_SIZE) % DATA_HISTORY_SIZE;
        if (dataHistory[index].timestamp >= startTime && dataHistory[index].timestamp <= endTime) {
            JsonObject point = dataArray.add<JsonObject>();
            
            char dateStr[11];
            char timeStr[9];
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&dataHistory[index].timestamp));
            strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&dataHistory[index].timestamp));
            
            point["date"] = dateStr;
            point["time"] = timeStr;
            point["temp"] = dataHistory[index].temperature;
            point["compressor"] = dataHistory[index].compressorState;
            point["defrost"] = dataHistory[index].defrostState;
            point["fan"] = dataHistory[index].fanState;
            point["remainingDefrostTime"] = dataHistory[index].remainingDefrostTime;
            point["remainingDripTime"] = dataHistory[index].remainingDripTime;
        }
    }

    String response;
    serializeJson(doc, response);
    return response;
}