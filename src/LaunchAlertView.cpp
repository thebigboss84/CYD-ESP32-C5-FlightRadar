#include "LaunchAlertView.h"
#include "DisplayEngine.h"
#include <math.h>

void LaunchAlertView::draw(const SpaceXRecord &launch, int64_t secondsToLaunch) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);
  gfx->fillRoundRect(12, CONTENT_Y + 12, SCREEN_W - 24, CONTENT_H - 24, 8, 0x2104);
  gfx->drawRoundRect(12, CONTENT_Y + 12, SCREEN_W - 24, CONTENT_H - 24, 8, COL_ORANGE);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_ORANGE);
  gfx->setCursor(84, CONTENT_Y + 25);
  gfx->print("SPACEX LAUNCH NOTICE");

  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(31, CONTENT_Y + 47);
  gfx->print("LAUNCH WINDOW OPENS IN");

  int hours = (int)(secondsToLaunch / 3600LL);
  int minutes = (int)((secondsToLaunch % 3600LL) / 60LL);
  int seconds = (int)(secondsToLaunch % 60LL);
  char countdown[24];
  snprintf(countdown, sizeof(countdown), "T - %02d:%02d:%02d", hours, minutes, seconds);

  gfx->setTextSize(3);
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(48, CONTENT_Y + 62);
  gfx->print(countdown);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(32, CONTENT_Y + 108);
  gfx->printf("MISSION: %.34s", launch.mission_name);
  gfx->setCursor(32, CONTENT_Y + 123);
  gfx->printf("VEHICLE: %.34s", launch.rocket_name);
  gfx->setCursor(32, CONTENT_Y + 138);
  gfx->printf("SITE: %.37s", launch.location_name);

  gfx->fillRoundRect(104, CONTENT_Y + 158, 112, 24, 5, COL_ORANGE);
  gfx->drawRoundRect(104, CONTENT_Y + 158, 112, 24, 5, COL_YELLOW);
  gfx->setTextColor(COL_BLACK);
  gfx->setCursor(135, CONTENT_Y + 166);
  gfx->print("CLOSE");
}

bool LaunchAlertView::isCloseTouched(int tx, int ty) {
  return tx >= 104 && tx <= 216 && ty >= CONTENT_Y + 158 && ty <= CONTENT_Y + 182;
}
