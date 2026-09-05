#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>
#include "AppConfig.h"

class DisplayEngine {
public:
  static void begin();
  static Arduino_GFX *getGfx();
  static bool readTouch(int &touchX, int &touchY);
  static void drawHeader(const char *city, const char *timeStr, int flightCount, bool wifiOk);
  static void drawFooter(AppMode activeMode);
  static void showStatus(const char *msg, uint16_t color = COL_CYAN);
  static void drawWeatherIcon(int x, int y, int code, uint16_t color);
  static void drawMiniCompass(int cx, int cy, int r, float bearing, uint16_t color);
  static void setBrightness(uint8_t duty);

  static Arduino_DataBus *bus;
  static Arduino_GFX     *gfx;
  static XPT2046_Touchscreen ts;
};
