#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class CitySelectView {
public:
  static void draw();
  static bool handleTouch(int tx, int ty);

  static bool isUsingGps();
  static void setUseGps(bool use);

private:
  static bool usingGps;
};
