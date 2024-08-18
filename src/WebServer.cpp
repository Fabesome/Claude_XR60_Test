#include "WebServer.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "Settings.h"
#include "Hardware.h"
#include "DataLogger.h"
#include "config.h"

AsyncWebServer server(80);

void setupWebServer() {
  // Serve the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Get current temperature
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["main"] = currentTemperature;
    if (settings.P2P == "y") {
      doc["evaporator"] = evaporatorTemperature;
    }
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Serve the parameters page
  server.on("/parameters", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/parameters.html", "text/html");
  });

  // Calibrate sensor
  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      
      if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
      }
      
      float temp1 = doc["temp1"];
      float temp2 = doc["temp2"];
      float temp3 = doc["temp3"];
      
      calibrateSensor(temp1, temp2, temp3);
      
      request->send(200, "text/plain", "Calibration complete");
    }
  );



  // Get latest data
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", getLatestDataJSON());
  });

  // Get data for a specific time range
  server.on("/data_range", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("start") && request->hasParam("end")) {
      unsigned long startTime = request->getParam("start")->value().toInt();
      unsigned long endTime = request->getParam("end")->value().toInt();
      request->send(200, "application/json", getDataJSON(startTime, endTime));
    } else {
      request->send(400, "text/plain", "Missing start or end parameters");
    }
  });

  // Get alert status
  server.on("/alert_status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["highTemp"] = highTempAlert;
    doc["lowTemp"] = lowTempAlert;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Download log file
  server.on("/download_log", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, DATA_FILE, "text/csv", true);
  });

  // Simulate temperature
  server.on("/simulate_temperature", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      
      if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
      }
      
      if (doc.containsKey("temperature")) {
        simulatedTemperature = doc["temperature"];
        useSimulatedTemperature = true;
        request->send(200, "application/json", "{\"status\":\"success\"}");
      } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing temperature value\"}");
      }
    }
  );

  // Update settings
  server.on("/update_settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      
      if (error) {
        request->send(400, "text/plain", "Invalid JSON");
        return;
      }
      
      if (doc.containsKey("SEt")) settings.SEt = doc["SEt"];
      if (doc.containsKey("Hy")) settings.Hy = doc["Hy"];
      if (doc.containsKey("LS")) settings.LS = doc["LS"];
      if (doc.containsKey("US")) settings.US = doc["US"];
      if (doc.containsKey("Ot")) settings.Ot = doc["Ot"];
      if (doc.containsKey("P2P")) settings.P2P = doc["P2P"].as<String>();
      if (doc.containsKey("OE")) settings.OE = doc["OE"];
      if (doc.containsKey("OdS")) settings.OdS = doc["OdS"];
      if (doc.containsKey("AC")) settings.AC = doc["AC"];
      if (doc.containsKey("CCt")) settings.CCt = doc["CCt"];
      if (doc.containsKey("CCS")) settings.CCS = doc["CCS"];
      if (doc.containsKey("COn")) settings.COn = doc["COn"];
      if (doc.containsKey("COF")) settings.COF = doc["COF"];
      if (doc.containsKey("CF")) settings.CF = doc["CF"].as<String>();
      if (doc.containsKey("rES")) settings.rES = doc["rES"].as<String>();
      if (doc.containsKey("Lod")) settings.Lod = doc["Lod"].as<String>();
      if (doc.containsKey("tdF")) settings.tdF = doc["tdF"].as<String>();
      if (doc.containsKey("dFP")) settings.dFP = doc["dFP"].as<String>();
      if (doc.containsKey("dtE")) settings.dtE = doc["dtE"];
      if (doc.containsKey("IdF")) settings.IdF = doc["IdF"];
      if (doc.containsKey("MdF")) settings.MdF = doc["MdF"];
      if (doc.containsKey("dSd")) settings.dSd = doc["dSd"];
      if (doc.containsKey("dFd")) settings.dFd = doc["dFd"].as<String>();
      if (doc.containsKey("dAd")) settings.dAd = doc["dAd"];
      if (doc.containsKey("Fdt")) settings.Fdt = doc["Fdt"];
      if (doc.containsKey("dPo")) settings.dPo = doc["dPo"].as<String>();
      if (doc.containsKey("dAF")) settings.dAF = doc["dAF"];
      if (doc.containsKey("FnC")) settings.FnC = doc["FnC"].as<String>();
      if (doc.containsKey("Fnd")) settings.Fnd = doc["Fnd"];
      if (doc.containsKey("Fct")) settings.Fct = doc["Fct"];
      if (doc.containsKey("FSt")) settings.FSt = doc["FSt"];
      if (doc.containsKey("FAP")) settings.FAP = doc["FAP"].as<String>();
      if (doc.containsKey("ALC")) settings.ALC = doc["ALC"].as<String>();
      if (doc.containsKey("ALU")) settings.ALU = doc["ALU"];
      if (doc.containsKey("ALL")) settings.ALL = doc["ALL"];
      if (doc.containsKey("AFH")) settings.AFH = doc["AFH"];
      if (doc.containsKey("ALd")) settings.ALd = doc["ALd"];
      if (doc.containsKey("dAO")) settings.dAO = doc["dAO"];
      if (doc.containsKey("useBME280")) settings.useBME280 = doc["useBME280"];

      saveSettings();
      request->send(200, "application/json", "{\"status\":\"success\"}");
    }
  );

  // Get current settings
  server.on("/get_settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["SEt"] = settings.SEt;
    doc["Hy"] = settings.Hy;
    doc["LS"] = settings.LS;
    doc["US"] = settings.US;
    doc["Ot"] = settings.Ot;
    doc["P2P"] = settings.P2P;
    doc["OE"] = settings.OE;
    doc["OdS"] = settings.OdS;
    doc["AC"] = settings.AC;
    doc["CCt"] = settings.CCt;
    doc["CCS"] = settings.CCS;
    doc["COn"] = settings.COn;
    doc["COF"] = settings.COF;
    doc["CF"] = settings.CF;
    doc["rES"] = settings.rES;
    doc["Lod"] = settings.Lod;
    doc["tdF"] = settings.tdF;
    doc["dFP"] = settings.dFP;
    doc["dtE"] = settings.dtE;
    doc["IdF"] = settings.IdF;
    doc["MdF"] = settings.MdF;
    doc["dSd"] = settings.dSd;
    doc["dFd"] = settings.dFd;
    doc["dAd"] = settings.dAd;
    doc["Fdt"] = settings.Fdt;
    doc["dPo"] = settings.dPo;
    doc["dAF"] = settings.dAF;
    doc["FnC"] = settings.FnC;
    doc["Fnd"] = settings.Fnd;
    doc["Fct"] = settings.Fct;
    doc["FSt"] = settings.FSt;
    doc["FAP"] = settings.FAP;
    doc["ALC"] = settings.ALC;
    doc["ALU"] = settings.ALU;
    doc["ALL"] = settings.ALL;
    doc["AFH"] = settings.AFH;
    doc["ALd"] = settings.ALd;
    doc["dAO"] = settings.dAO;
    doc["useBME280"] = settings.useBME280;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Toggle sensor type
  server.on("/toggle_sensor", HTTP_POST, [](AsyncWebServerRequest *request) {
    settings.useBME280 = !settings.useBME280;
    saveSettings();
    String response = settings.useBME280 ? "Using BME280" : "Using NTC";
    request->send(200, "text/plain", response);
  });

  // Start the server
  server.begin();
}