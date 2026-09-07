#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class SeismicView {
public:
  static void draw();
  static bool handleTouch(int tx, int ty);

private:
  static void drawSeismogram(int x, int y, int w, int h, float mag);
};
