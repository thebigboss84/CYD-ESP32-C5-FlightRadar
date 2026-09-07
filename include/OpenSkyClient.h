#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class OpenSkyClient {
public:
  static bool fetch(float userLat, float userLon, float radiusKm);
  static int  getCount();
  static const FlightRecord *getFlight(int index);
  static const char *getCompassDir(float bearing);
  static float metersToFeet(float m);
  static float msToKnots(float ms);
  static void  fetchRoute(int index);
  static void  extrapolatePositions(float dtSeconds, float userLat, float userLon);

  // Emergency & Proximity queries
  static bool hasActiveEmergency();
  static const FlightRecord *getEmergencyFlight();
  static bool hasAircraftOverhead(float thresholdKm = 6.0f);

private:
  static void insertSorted(const FlightRecord &rec);
  static float calcHaversine(float lat1, float lon1, float lat2, float lon2);
  static float calcBearing(float lat1, float lon1, float lat2, float lon2);

  static FlightRecord flights[MAX_TRACKED_FLIGHTS];
  static int flightCount;
};
