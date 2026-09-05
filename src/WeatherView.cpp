#include "WeatherView.h"
#include "DisplayEngine.h"
#include "WeatherClient.h"

void WeatherView::draw(const char *cityName) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  const WeatherRecord &wx = WeatherClient::getData();
  if (!wx.valid) {
    gfx->setTextColor(COL_GRAY);
    gfx->setTextSize(1);
    gfx->setCursor(80, CONTENT_Y + 80);
    gfx->print("Awaiting weather telemetry...");
    return;
  }

  gfx->setTextSize(2);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(10, CONTENT_Y + 8);
  gfx->printf("%.1f C", wx.temp_c);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(104, CONTENT_Y + 14);
  gfx->printf("Feels: %.1f C", wx.feels_like_c);

  DisplayEngine::drawWeatherIcon(260, CONTENT_Y + 6, wx.weather_code, COL_YELLOW);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(10, CONTENT_Y + 34);
  gfx->printf("%s", WeatherClient::getConditionText(wx.weather_code));

  gfx->drawFastHLine(10, CONTENT_Y + 48, SCREEN_W - 20, COL_DIM_GRAY);

  gfx->setCursor(10, CONTENT_Y + 54);
  gfx->setTextColor(COL_GRAY); gfx->print("HUMIDITY: ");
  gfx->setTextColor(COL_WHITE); gfx->printf("%d%%     ", wx.humidity);

  gfx->setTextColor(COL_GRAY); gfx->print("WIND: ");
  gfx->setTextColor(COL_WHITE); gfx->printf("%.1f km/h (%.0f deg)", wx.wind_speed_kmh, wx.wind_dir_deg);

  gfx->drawFastHLine(10, CONTENT_Y + 70, SCREEN_W - 20, COL_GRAY);

  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(10, CONTENT_Y + 76);
  gfx->print("4-DAY FORECAST");

  int cardW = 70;
  int cardH = 88;
  int startX = 10;
  int startY = CONTENT_Y + 92;

  for (int i = 0; i < 4; i++) {
    int cx = startX + (i * 75);
    gfx->fillRect(cx, startY, cardW, cardH, 0x0006);
    gfx->drawRect(cx, startY, cardW, cardH, COL_DIM_GRAY);

    gfx->setTextColor(COL_CYAN);
    gfx->setTextSize(1);
    gfx->setCursor(cx + 12, startY + 6);
    gfx->printf("DAY %s", wx.daily_day[i]);

    DisplayEngine::drawWeatherIcon(cx + 18, startY + 20, wx.daily_code[i], COL_YELLOW);

    gfx->setTextColor(COL_WHITE);
    gfx->setCursor(cx + 8, startY + 58);
    gfx->printf("H:%.0f C", wx.daily_max[i]);

    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(cx + 8, startY + 72);
    gfx->printf("L:%.0f C", wx.daily_min[i]);
  }
}
