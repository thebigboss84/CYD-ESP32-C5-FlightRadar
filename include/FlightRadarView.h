#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class FlightRadarView {
public:
  static void draw(float radiusKm);
  static void updateSweep(float radiusKm);

private:
  static float sweepAngle;
};
