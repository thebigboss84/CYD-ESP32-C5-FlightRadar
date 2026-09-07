#pragma once

#include <Arduino.h>
#include "AppConfig.h"

#define MAX_SEISMIC_EVENTS 5

class SeismicClient {
public:
  static bool fetch(float userLat, float userLon);
  static bool hasActiveAlert();
  static const SeismicRecord &getLatest();
  static int getCount();
  static const SeismicRecord *getRecord(int index);

  static float calculateDistanceKm(float lat1, float lon1, float lat2, float lon2);
  static float calculateBearingDeg(float lat1, float lon1, float lat2, float lon2);

private:
  static SeismicRecord records[MAX_SEISMIC_EVENTS];
  static int recordCount;
  static bool alertActive;
};
