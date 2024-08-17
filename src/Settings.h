#pragma once

#include <Arduino.h>

struct Settings {
  float SEt;  // Set Point
  float Hy;   // Differential
  float LS;   // Minimum Set Point
  float US;   // Maximum Set Point
  float Ot;   // Thermostat Probe Calibration
  String P2P; // Evaporator Probe Presence
  float OE;   // Evaporator Probe Calibration
  int OdS;    // Outputs Activation Delay at Start Up
  int AC;     // Anti-short Cycle Delay
  float CCt;  // Continuous Cycle Duration
  float CCS;  // Set Point for Continuous Cycle
  int COn;    // Compressor ON Time with Faulty Probe
  int COF;    // Compressor OFF Time with Faulty Probe
  String CF;  // Temperature Measurement Unit
  String rES; // Resolution
  String Lod; // Probe Displayed
  String tdF; // Defrost Type
  String dFP; // Probe Selection for Defrost Termination
  float dtE;  // Defrost Termination Temperature
  int IdF;    // Interval Between Defrost Cycles
  int MdF;    // Maximum Duration of Defrost
  int dSd;    // Start Defrost Delay
  String dFd; // Display During Defrost
  int dAd;    // MAX Display Delay After Defrost
  int Fdt;    // Draining Time
  String dPo; // First Defrost After Start-up
  float dAF;  // Defrost Delay After Continuous Cycle
  String FnC; // Fan Operating Mode
  int Fnd;    // Fan Delay After Defrost
  float Fct;  // Temperature Differential to Avoid Short Cycles of Fans
  float FSt;  // Fan Stop Temperature
  String FAP; // Probe Selection for Fan Management
  String ALC; // Temperature Alarms Configuration
  float ALU;  // High Temperature Alarm Setting
  float ALL;  // Low Temperature Alarm Setting
  float AFH;  // Differential for Temperature Alarm Recovery
  int ALd;    // Temperature Alarm Delay
  float dAO;  // Delay of Temperature Alarm at Start Up
  bool useBME280; // Use BME280 sensor instead of NTC
};

extern Settings settings;

void loadSettings();
void saveSettings();