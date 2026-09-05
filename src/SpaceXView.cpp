#include "SpaceXView.h"
#include "DisplayEngine.h"
#include "SpaceXClient.h"
#include <time.h>

void SpaceXView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  const SpaceXRecord &sp = SpaceXClient::getData();
  if (!sp.valid) {
    gfx->setTextColor(COL_GRAY);
    gfx->setTextSize(1);
    gfx->setCursor(60, CONTENT_Y + 80);
    gfx->print("Retrieving SpaceX launch manifest...");
    return;
  }

  gfx->setTextSize(1);
  gfx->setTextColor(COL_ORANGE);
  gfx->setCursor(10, CONTENT_Y + 6);
  gfx->print("SPACEX LAUNCH OPERATION");
  gfx->drawFastHLine(10, CONTENT_Y + 18, SCREEN_W - 20, COL_DIM_GRAY);

  gfx->setTextColor(COL_WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(10, CONTENT_Y + 24);
  gfx->print("MISSION: ");
  gfx->setTextColor(COL_YELLOW);
  char mName[38];
  snprintf(mName, sizeof(mName), "%s", sp.mission_name);
  gfx->print(mName);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, CONTENT_Y + 38);
  gfx->print("VEHICLE: ");
  gfx->setTextColor(COL_CYAN);
  char rName[38];
  snprintf(rName, sizeof(rName), "%s", sp.rocket_name);
  gfx->print(rName);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, CONTENT_Y + 52);
  gfx->print("PAD:     ");
  gfx->setTextColor(COL_WHITE);
  char pName[38];
  snprintf(pName, sizeof(pName), "%s", sp.pad_name);
  gfx->print(pName);

  int cdBoxY = CONTENT_Y + 68;
  gfx->fillRect(10, cdBoxY, SCREEN_W - 20, 44, 0x0842);
  gfx->drawRect(10, cdBoxY, SCREEN_W - 20, 44, COL_ORANGE);

  gfx->setTextColor(COL_ORANGE);
  gfx->setCursor(18, cdBoxY + 5);
  gfx->print("T-MINUS COUNTDOWN:");

  time_t nowUtc = time(nullptr);
  int64_t diffSec = sp.launch_epoch_utc - (int64_t)nowUtc;

  char cdBuf[32];
  if (diffSec > 0) {
    int days  = (int)(diffSec / 86400LL);
    int hours = (int)((diffSec % 86400LL) / 3600LL);
    int mins  = (int)((diffSec % 3600LL) / 60LL);
    int secs  = (int)(diffSec % 60LL);
    snprintf(cdBuf, sizeof(cdBuf), "T - %02dd %02d:%02d:%02d", days, hours, mins, secs);
  } else if (diffSec > -3600LL) {
    snprintf(cdBuf, sizeof(cdBuf), "LIFTOFF / IN FLIGHT");
  } else {
    snprintf(cdBuf, sizeof(cdBuf), "TARGET: %s", sp.net_iso);
  }

  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(24, cdBoxY + 20);
  gfx->print(cdBuf);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, CONTENT_Y + 120);
  gfx->print("STATUS:  ");
  gfx->setTextColor(COL_GREEN);
  gfx->print(sp.status_name);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, CONTENT_Y + 134);
  gfx->print("TARGET:  ");
  gfx->setTextColor(COL_CYAN);
  gfx->print(sp.net_iso);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, CONTENT_Y + 150);
  char detBuf[54];
  snprintf(detBuf, sizeof(detBuf), "%.50s", sp.details);
  gfx->print(detBuf);
}

void SpaceXView::updateCountdown() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const SpaceXRecord &sp = SpaceXClient::getData();
  if (!sp.valid || sp.launch_epoch_utc == 0) return;

  int cdBoxY = CONTENT_Y + 68;
  time_t nowUtc = time(nullptr);
  int64_t diffSec = sp.launch_epoch_utc - (int64_t)nowUtc;

  char cdBuf[32];
  if (diffSec > 0) {
    int days  = (int)(diffSec / 86400LL);
    int hours = (int)((diffSec % 86400LL) / 3600LL);
    int mins  = (int)((diffSec % 3600LL) / 60LL);
    int secs  = (int)(diffSec % 60LL);
    snprintf(cdBuf, sizeof(cdBuf), "T - %02dd %02d:%02d:%02d", days, hours, mins, secs);
  } else {
    snprintf(cdBuf, sizeof(cdBuf), "LIFTOFF / IN FLIGHT");
  }

  gfx->fillRect(20, cdBoxY + 20, 276, 18, 0x0842);
  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(24, cdBoxY + 20);
  gfx->print(cdBuf);
}
