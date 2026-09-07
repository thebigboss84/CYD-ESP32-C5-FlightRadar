#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class FlightRadarView {
public:
  static void draw(float defaultRadiusKm = 100.0f);
  static void updateSweep(float defaultRadiusKm = 100.0f);
  static bool handleTouch(int tx, int ty);
  static bool checkSeismicBannerTouch(int tx, int ty);

  static void cycleZoom();
  static void cycleFilter();
  static float getZoomRadius();
  static TrafficFilter getActiveFilter();

private:
  static float sweepAngle;
  static float currentRadiusKm;
  static TrafficFilter activeFilter;
};
