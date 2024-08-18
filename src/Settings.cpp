#include "Settings.h"
#include <Preferences.h>

Settings settings;
Preferences preferences;

void loadSettings() {
  preferences.begin("refrigCtrl", true);

  settings.SEt = preferences.getFloat("SEt", -5.0);
  settings.Hy = preferences.getFloat("Hy", 2.0);
  settings.LS = preferences.getFloat("LS", -50.0);
  settings.US = preferences.getFloat("US", 110.0);
  settings.Ot = preferences.getFloat("Ot", 0.0);
  settings.P2P = preferences.getString("P2P", "y");
  settings.OE = preferences.getFloat("OE", 0.0);
  settings.OdS = preferences.getInt("OdS", 0);
  settings.AC = preferences.getInt("AC", 1);
  settings.CCt = preferences.getFloat("CCt", 0.0);
  settings.CCS = preferences.getFloat("CCS", -5.0);
  settings.COn = preferences.getInt("COn", 15);
  settings.COF = preferences.getInt("COF", 30);
  settings.CF = preferences.getString("CF", "C");
  settings.rES = preferences.getString("rES", "dE");
  settings.Lod = preferences.getString("Lod", "P1");
  settings.tdF = preferences.getString("tdF", "EL");
  settings.dFP = preferences.getString("dFP", "P2");
  settings.dtE = preferences.getFloat("dtE", 8.0);
  settings.IdF = preferences.getInt("IdF", 6);
  settings.MdF = preferences.getInt("MdF", 30);
  settings.dSd = preferences.getInt("dSd", 0);
  settings.dFd = preferences.getString("dFd", "it");
  settings.dAd = preferences.getInt("dAd", 30);
  settings.Fdt = preferences.getInt("Fdt", 0);
  settings.dPo = preferences.getString("dPo", "n");
  settings.dAF = preferences.getFloat("dAF", 0.0);
  settings.FnC = preferences.getString("FnC", "o-n");
  settings.Fnd = preferences.getInt("Fnd", 10);
  settings.Fct = preferences.getFloat("Fct", 10.0);
  settings.FSt = preferences.getFloat("FSt", 2.0);
  settings.FAP = preferences.getString("FAP", "P2");
  settings.ALC = preferences.getString("ALC", "Ab");
  settings.ALU = preferences.getFloat("ALU", 110.0);
  settings.ALL = preferences.getFloat("ALL", -50.0);
  settings.AFH = preferences.getFloat("AFH", 1.0);
  settings.ALd = preferences.getInt("ALd", 15);
  settings.dAO = preferences.getFloat("dAO", 1.3);
  settings.HES = preferences.getFloat("HES", 0.0);
  settings.useBME280 = preferences.getBool("useBME280", false);

  preferences.end();
}

void saveSettings() {
  preferences.begin("refrigCtrl", false);

  preferences.putFloat("SEt", settings.SEt);
  preferences.putFloat("Hy", settings.Hy);
  preferences.putFloat("LS", settings.LS);
  preferences.putFloat("US", settings.US);
  preferences.putFloat("Ot", settings.Ot);
  preferences.putString("P2P", settings.P2P);
  preferences.putFloat("OE", settings.OE);
  preferences.putInt("OdS", settings.OdS);
  preferences.putInt("AC", settings.AC);
  preferences.putFloat("CCt", settings.CCt);
  preferences.putFloat("CCS", settings.CCS);
  preferences.putInt("COn", settings.COn);
  preferences.putInt("COF", settings.COF);
  preferences.putString("CF", settings.CF);
  preferences.putString("rES", settings.rES);
  preferences.putString("Lod", settings.Lod);
  preferences.putString("tdF", settings.tdF);
  preferences.putString("dFP", settings.dFP);
  preferences.putFloat("dtE", settings.dtE);
  preferences.putInt("IdF", settings.IdF);
  preferences.putInt("MdF", settings.MdF);
  preferences.putInt("dSd", settings.dSd);
  preferences.putString("dFd", settings.dFd);
  preferences.putInt("dAd", settings.dAd);
  preferences.putInt("Fdt", settings.Fdt);
  preferences.putString("dPo", settings.dPo);
  preferences.putFloat("dAF", settings.dAF);
  preferences.putString("FnC", settings.FnC);
  preferences.putInt("Fnd", settings.Fnd);
  preferences.putFloat("Fct", settings.Fct);
  preferences.putFloat("FSt", settings.FSt);
  preferences.putString("FAP", settings.FAP);
  preferences.putString("ALC", settings.ALC);
  preferences.putFloat("ALU", settings.ALU);
  preferences.putFloat("ALL", settings.ALL);
  preferences.putFloat("AFH", settings.AFH);
  preferences.putInt("ALd", settings.ALd);
  preferences.putFloat("dAO", settings.dAO);
  preferences.putFloat("HES", settings.HES);
  preferences.putBool("useBME280", settings.useBME280);

  preferences.end();
}