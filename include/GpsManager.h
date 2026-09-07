#pragma once

#include "AppConfig.h"

class GpsManager {
public:
  static void begin();
  static void update();

  static bool  hasFix();
  static float getLat();
  static float getLon();
  static float getAltitudeM();
  static int   getSatellites();
  static float getSpeedKmh();
  static float getHdop();
  static unsigned long getLastFixMs();

  // Returns true if GPS has moved more than thresholdKm since last marked position
  static bool hasMovedSignificantly(float thresholdKm = 3.0f);
  static void markReportedPosition();

  static const GpsData &getData();

private:
  static GpsData data;
  static float lastReportedLat;
  static float lastReportedLon;
  static bool  hasReportedOnce;

  static void parseNmeaSentence(const char *sentence);
  static float parseNmeaCoord(const char *str, char dir);
  static bool validateChecksum(const char *sentence);
};
