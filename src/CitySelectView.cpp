#include "CitySelectView.h"
#include "DisplayEngine.h"
#include "ConfigPortal.h"
#include "GpsManager.h"

bool CitySelectView::usingGps = false;

bool CitySelectView::isUsingGps() {
  return usingGps;
}

void CitySelectView::setUseGps(bool use) {
  usingGps = use;
}

void CitySelectView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  // Deep aerospace dark background
  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  // 1. Top Section: Active Airspace & GPS Telemetry Card (y: CONTENT_Y + 3 .. CONTENT_Y + 53)
  int topBoxY = CONTENT_Y + 3;
  int topBoxH = 50;
  gfx->fillRoundRect(4, topBoxY, SCREEN_W - 8, topBoxH, 4, 0x0842);
  gfx->drawRoundRect(4, topBoxY, SCREEN_W - 8, topBoxH, 4, 0x0269);

  // Vertical divider between Airspace and GPS Info
  gfx->drawFastVLine(182, topBoxY + 3, topBoxH - 6, 0x0185);

  // Left side: Active Location
  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, topBoxY + 5);
  gfx->print("CURRENT TARGET:");

  gfx->setTextSize(2);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(10, topBoxY + 16);
  if (usingGps && GpsManager::hasFix()) {
    gfx->print("GPS Live");
  } else {
    char nameBuf[16];
    strncpy(nameBuf, ConfigPortal::getCityName(), sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    gfx->print(nameBuf);
  }

  gfx->setTextSize(1);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(10, topBoxY + 35);
  float curLat = (usingGps && GpsManager::hasFix()) ? GpsManager::getLat() : ConfigPortal::getLat();
  float curLon = (usingGps && GpsManager::hasFix()) ? GpsManager::getLon() : ConfigPortal::getLon();
  gfx->printf("Lat:%.3f Lon:%.3f (%.0fkm)", curLat, curLon, ConfigPortal::getRadius());

  // Right side: GPS Telemetry
  gfx->setTextSize(1);
  gfx->setTextColor(0x8410);
  gfx->setCursor(188, topBoxY + 5);
  gfx->print("GPS MODULE (P5):");

  if (GpsManager::hasFix()) {
    gfx->setTextColor(0x07E0); // Bright Green
    gfx->setCursor(188, topBoxY + 16);
    gfx->printf("3D FIX (%dsat)", GpsManager::getSatellites());

    gfx->setTextColor(COL_WHITE);
    gfx->setCursor(188, topBoxY + 26);
    gfx->printf("HDOP: %.1f", GpsManager::getHdop());

    gfx->setTextColor(COL_CYAN);
    gfx->setCursor(188, topBoxY + 36);
    gfx->printf("%.3f, %.3f", GpsManager::getLat(), GpsManager::getLon());
  } else {
    gfx->setTextColor(COL_YELLOW);
    gfx->setCursor(188, topBoxY + 16);
    gfx->print("SEARCHING SATS...");

    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(188, topBoxY + 26);
    gfx->printf("Sats: %d (LP-UART)", GpsManager::getSatellites());

    gfx->setTextColor(0x632C);
    gfx->setCursor(188, topBoxY + 36);
    gfx->print("Acquiring 3D fix");
  }

  // 2. Section Header: Presets Selection
  gfx->setTextColor(COL_YELLOW);
  gfx->setTextSize(1);
  gfx->setCursor(6, CONTENT_Y + 56);
  gfx->print("SELECT AIRSPACE (TAP CARD TO SWITCH & UPDATE):");

  // 3. Grid of 10 Selection Cards (2 columns x 5 rows)
  // Row Y offsets: 66, 91, 116, 141, 166 (each height 22, step 25)
  const int colW = 153;
  const int cardH = 22;
  const int startY = CONTENT_Y + 66;
  const int stepY = 25;

  for (int slot = 0; slot < 10; slot++) {
    int col = slot % 2;
    int row = slot / 2;
    int cx = (col == 0) ? 4 : 163;
    int cy = startY + row * stepY;

    bool isActive = false;
    char title[24];
    char sub[32];

    if (slot == 0) {
      // Slot 0: Live GPS
      isActive = usingGps;
      strncpy(title, "[GPS] Live Fix", sizeof(title));
      if (GpsManager::hasFix()) {
        snprintf(sub, sizeof(sub), "%.2f, %.2f (%dsat)", GpsManager::getLat(), GpsManager::getLon(), GpsManager::getSatellites());
      } else {
        strncpy(sub, "Search Satellites", sizeof(sub));
      }
    } else if (slot == 1) {
      // Slot 1: Home / Custom
      isActive = (!usingGps && ConfigPortal::isCustomCity());
      snprintf(title, sizeof(title), "[HOME] %s", ConfigPortal::getCustomCityName());
      snprintf(sub, sizeof(sub), "%.2f, %.2f (NVS)", ConfigPortal::getLat(), ConfigPortal::getLon());
    } else {
      // Slots 2..9: Presets 0..7
      int pIdx = slot - 2;
      const CityPreset &p = CITY_PRESETS[pIdx];
      isActive = (!usingGps && !ConfigPortal::isCustomCity() && ConfigPortal::getCityPresetIndex() == pIdx);
      strncpy(title, p.name, sizeof(title));
      snprintf(sub, sizeof(sub), "%.2f, %.2f | %s", p.lat, p.lon, p.subtitle);
    }

    // Card background and border
    if (isActive) {
      gfx->fillRoundRect(cx, cy, colW, cardH, 3, 0x028A); // Distinct tactical teal background
      gfx->drawRoundRect(cx, cy, colW, cardH, 3, COL_CYAN); // Bright cyan highlight
      gfx->drawRoundRect(cx + 1, cy + 1, colW - 2, cardH - 2, 2, COL_CYAN); // Double border
    } else {
      gfx->fillRoundRect(cx, cy, colW, cardH, 3, 0x0842); // Dark slate
      gfx->drawRoundRect(cx, cy, colW, cardH, 3, 0x0269); // Subtle border
    }

    // Text rendering inside card
    gfx->setTextSize(1);
    if (isActive) {
      gfx->setTextColor(COL_WHITE);
      gfx->setCursor(cx + 4, cy + 3);
      gfx->print(title);

      gfx->setTextColor(0x07E0); // Bright green active tag
      gfx->setCursor(cx + colW - 30, cy + 3);
      gfx->print("[ACT]");

      gfx->setTextColor(COL_CYAN);
      gfx->setCursor(cx + 4, cy + 12);
      gfx->print(sub);
    } else {
      gfx->setTextColor(COL_WHITE);
      gfx->setCursor(cx + 4, cy + 3);
      gfx->print(title);

      gfx->setTextColor(COL_GRAY);
      gfx->setCursor(cx + 4, cy + 12);
      gfx->print(sub);
    }
  }
}

bool CitySelectView::handleTouch(int tx, int ty) {
  const int startY = CONTENT_Y + 66;
  const int stepY = 25;
  const int cardH = 22;

  // Check if touch is within the button grid area
  if (ty < startY || ty > startY + 5 * stepY) return false;

  int row = (ty - startY) / stepY;
  if (row < 0 || row >= 5) return false;

  int col = (tx < 160) ? 0 : 1;
  int slot = row * 2 + col;

  if (slot == 0) {
    Serial.println("[CitySelect] Switched to: LIVE GPS FIX");
    setUseGps(true);
  } else if (slot == 1) {
    Serial.printf("[CitySelect] Switched to: HOME (%s)\n", ConfigPortal::getCustomCityName());
    setUseGps(false);
    ConfigPortal::setCustomCity();
  } else {
    int pIdx = slot - 2;
    if (pIdx >= 0 && pIdx < (int)CITY_PRESETS_COUNT) {
      Serial.printf("[CitySelect] Switched to Preset: %s\n", CITY_PRESETS[pIdx].name);
      setUseGps(false);
      ConfigPortal::setCityPreset(pIdx);
    }
  }

  draw();
  DisplayEngine::showStatus("Airspace updated! Re-fetching...", COL_GREEN);
  return true;
}
