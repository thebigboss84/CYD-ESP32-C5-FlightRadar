#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class IssClient {
public:
  static bool fetch(float userLat, float userLon);
  static const IssRecord &getData();

private:
  static IssRecord iss;
  static float calcHaversine(float lat1, float lon1, float lat2, float lon2);
  static float calcBearing(float lat1, float lon1, float lat2, float lon2);
};
