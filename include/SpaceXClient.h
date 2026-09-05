#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class SpaceXClient {
public:
  static bool fetch();
  static const SpaceXRecord &getData();

private:
  static SpaceXRecord spaceX;
  static int64_t parseIsoToEpoch(const char *isoStr);
};
