#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class IssView {
public:
  static void draw(const char *cityName, float radarRadiusKm = 100.0f);
};
