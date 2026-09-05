#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class WeatherView {
public:
  static void draw(const char *cityName);
};
