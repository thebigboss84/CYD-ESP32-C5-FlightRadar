#include "SpaceXView.h"
#include "DisplayEngine.h"
#include "SpaceXClient.h"
#include <time.h>

static const char *getCompassStr(float deg) {
  const char *dirs[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                         "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
  int idx = (int)((deg + 11.25f) / 22.5f) % 16;
  if (idx < 0) idx += 16;
  return dirs[idx];
}

void SpaceXView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  const SpaceXRecord &sp = SpaceXClient::getData();
  if (!sp.valid) {
    gfx->setTextColor(COL_GRAY);
    gfx->setTextSize(1);
    gfx->setCursor(50, CONTENT_Y + 80);
    gfx->print("Retrieving SpaceX launch manifest...");
    return;
  }

  gfx->setTextSize(1);
  gfx->setTextColor(COL_ORANGE);
  gfx->setCursor(8, CONTENT_Y + 4);
  gfx->print("SPACEX MISSION TELEMETRY");
  gfx->drawFastHLine(8, CONTENT_Y + 14, SCREEN_W - 16, COL_DIM_GRAY);

  // Line 1: Mission Name
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 18);
  gfx->print("MISSION: ");
  gfx->setTextColor(COL_YELLOW);
  char mName[38];
  snprintf(mName, sizeof(mName), "%.36s", sp.mission_name);
  gfx->print(mName);

  // Line 2: Rocket Vehicle
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 30);
  gfx->print("VEHICLE: ");
  gfx->setTextColor(COL_CYAN);
  char rName[38];
  snprintf(rName, sizeof(rName), "%.36s", sp.rocket_name);
  gfx->print(rName);

  // Line 3: Launch Site Location
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 42);
  gfx->print("SITE:    ");
  gfx->setTextColor(COL_WHITE);
  char pName[38];
  snprintf(pName, sizeof(pName), "%.36s", sp.location_name);
  gfx->print(pName);

  // Line 4: Sky Visibility Badge (Crucial User Feature!)
  int visY = CONTENT_Y + 54;
  if (sp.dist_km > 0.0f) {
    if (sp.visible_in_sky) {
      // Local launch (e.g. Vandenberg from California) - Highlighted Green/Cyan
      gfx->fillRoundRect(8, visY, SCREEN_W - 16, 16, 3, 0x02E5);
      gfx->drawRoundRect(8, visY, SCREEN_W - 16, 16, 3, 0x07E0);
      gfx->setTextColor(0xFFFF);
      gfx->setCursor(14, visY + 4);
      gfx->printf("LOCAL SKY: %.0fkm %s - VISIBLE IN SKY!",
                  sp.dist_km, getCompassStr(sp.bearing));
    } else {
      // Distant launch (e.g. Cape Canaveral / Boca Chica) - Slate card
      gfx->fillRoundRect(8, visY, SCREEN_W - 16, 16, 3, 0x0842);
      gfx->drawRoundRect(8, visY, SCREEN_W - 16, 16, 3, 0x0269);
      gfx->setTextColor(0xAD55);
      gfx->setCursor(14, visY + 4);
      gfx->printf("SITE DIST: %.0fkm %s - BEYOND HORIZON",
                  sp.dist_km, getCompassStr(sp.bearing));
    }
  } else {
    gfx->setTextColor(COL_DIM_GRAY);
    gfx->setCursor(8, visY + 4);
    gfx->printf("PAD: %.36s", sp.pad_name);
  }

  // Countdown Box
  int cdBoxY = CONTENT_Y + 74;
  gfx->fillRoundRect(8, cdBoxY, SCREEN_W - 16, 42, 4, 0x0842);
  gfx->drawRoundRect(8, cdBoxY, SCREEN_W - 16, 42, 4, COL_ORANGE);

  gfx->setTextColor(COL_ORANGE);
  gfx->setCursor(16, cdBoxY + 5);
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
    snprintf(cdBuf, sizeof(cdBuf), "TARGET: %.16s", sp.net_iso);
  }

  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(20, cdBoxY + 19);
  gfx->print(cdBuf);

  // Status & Date Lines
  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 122);
  gfx->print("STATUS:  ");
  gfx->setTextColor(COL_GREEN);
  gfx->print(sp.status_name);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 135);
  gfx->print("TARGET:  ");
  gfx->setTextColor(COL_CYAN);
  gfx->print(sp.net_iso);

  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(8, CONTENT_Y + 150);
  char detBuf[54];
  snprintf(detBuf, sizeof(detBuf), "%.50s", sp.details);
  gfx->print(detBuf);
}

void SpaceXView::updateCountdown() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const SpaceXRecord &sp = SpaceXClient::getData();
  if (!sp.valid || sp.launch_epoch_utc == 0) return;

  int cdBoxY = CONTENT_Y + 74;
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
    return;
  }

  gfx->fillRect(16, cdBoxY + 19, 280, 18, 0x0842);
  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(20, cdBoxY + 19);
  gfx->print(cdBuf);
}
