#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class ConfigPortal {
public:
  static void loadSettings();
  static void saveSettings(const char *ssid, const char *pass, const char *city, float lat, float lon, float radius, bool isCustom = true);
  static void nextCityPreset();
  static void setCityPreset(int index);
  static void setCustomCity();

  static const char *getSsid();
  static const char *getPass();
  static const char *getCityName();
  static const char *getCustomCityName();
  static float getLat();
  static float getLon();
  static float getRadius();
  static bool  hasValidSettings();
  static bool  isCustomCity();
  static const char *getTimeZone();

  static void runPortal();
  static void stopPortal();

private:
  static char  ssid[64];
  static char  pass[64];
  static char  cityName[32];
  static float lat;
  static float lon;
  static float radiusKm;
  static int   currentCityIdx;
  static bool  configured;

  static char  customCityName[32];
  static float customLat;
  static float customLon;
  static bool  hasCustomCity;
  static bool  usingCustom;
};
