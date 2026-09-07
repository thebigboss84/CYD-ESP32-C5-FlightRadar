#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class SpaceXClient {
public:
  static bool fetch(float userLat = 0.0f, float userLon = 0.0f);
  static const SpaceXRecord &getData();

private:
  static SpaceXRecord spaceX;
  static int64_t parseIsoToEpoch(const char *isoStr);
};
