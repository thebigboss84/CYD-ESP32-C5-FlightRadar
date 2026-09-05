#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class WeatherClient {
public:
  static bool fetch(float lat, float lon);
  static const WeatherRecord &getData();
  static const char *getConditionText(int code);

private:
  static WeatherRecord weather;
};
