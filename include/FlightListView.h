#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class FlightListView {
public:
  static void draw();
  static void handleTouch(int tx, int ty);
  static void clearDetail();

private:
  static void drawRow(int rowIdx, bool isSelected);
  static void drawDetail(int index);

  static int selectedFlightIdx;
};
