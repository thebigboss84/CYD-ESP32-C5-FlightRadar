#include "FlightListView.h"
#include "DisplayEngine.h"
#include "OpenSkyClient.h"

int FlightListView::selectedFlightIdx = -1;

void FlightListView::clearDetail() {
  selectedFlightIdx = -1;
}

void FlightListView::drawRow(int rowIdx, bool isSelected) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const FlightRecord *f = OpenSkyClient::getFlight(rowIdx);
  if (!f) return;

  int y = CONTENT_Y + 16 + (rowIdx * LIST_ROW_HEIGHT);
  uint16_t rowBg = isSelected ? 0x0842 : ((rowIdx % 2 == 0) ? 0x0006 : COL_BG_DARK);

  gfx->fillRect(0, y, SCREEN_W, LIST_ROW_HEIGHT, rowBg);
  if (isSelected) {
    gfx->drawFastHLine(0, y, SCREEN_W, COL_CYAN);
    gfx->drawFastHLine(0, y + LIST_ROW_HEIGHT - 1, SCREEN_W, COL_CYAN);
  }

  uint16_t textColor = COL_AC_HIGH;
  if (f->on_ground) textColor = COL_AC_GND;
  else if (!isnan(f->alt_m) && f->alt_m < 3048.0f) textColor = COL_AC_LOW;

  gfx->setTextSize(1);

  // Callsign (0..68)
  gfx->setTextColor(textColor);
  gfx->setCursor(4, y + 6);
  gfx->print(f->callsign);

  // Route: Origin > Destination (68..145)
  gfx->setCursor(70, y + 6);
  if (strlen(f->origin) > 0 && strlen(f->dest) > 0) {
    gfx->setTextColor(COL_WHITE);
    gfx->printf("%-3s > %-3s", f->origin, f->dest);
  } else {
    gfx->setTextColor(COL_DIM_GRAY);
    gfx->print("  --- > ---");
  }

  // Altitude (146..205)
  gfx->setCursor(148, y + 6);
  gfx->setTextColor(textColor);
  if (f->on_ground) {
    gfx->print("   GND");
  } else if (isnan(f->alt_m)) {
    gfx->print("  ----");
  } else {
    gfx->printf("%5.0fft", OpenSkyClient::metersToFeet(f->alt_m));
  }

  // Speed (206..255)
  gfx->setCursor(210, y + 6);
  gfx->setTextColor(COL_GRAY);
  if (f->vel_ms > 1.0f) {
    gfx->printf("%3.0fkn", OpenSkyClient::msToKnots(f->vel_ms));
  } else {
    gfx->print("  0kn");
  }

  // Distance (256..318)
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(266, y + 6);
  gfx->printf("%4.0fkm", f->dist_km);
}

void FlightListView::drawDetail(int index) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  OpenSkyClient::fetchRoute(index);

  const FlightRecord *f = OpenSkyClient::getFlight(index);
  if (!f) return;

  int sheetH = 88;
  int sheetY = SCREEN_H - FOOTER_H - sheetH;

  gfx->fillRect(0, sheetY, SCREEN_W, sheetH, 0x0010);
  gfx->drawFastHLine(0, sheetY, SCREEN_W, COL_CYAN);

  gfx->setTextSize(2);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(8, sheetY + 6);
  gfx->print(f->callsign);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(120, sheetY + 10);
  gfx->printf("[%s] %s", f->icao, f->country);

  gfx->setCursor(8, sheetY + 28);
  gfx->setTextColor(COL_YELLOW); gfx->print("ROUTE: ");
  if (strlen(f->origin) > 0 && strlen(f->dest) > 0) {
    gfx->setTextColor(COL_WHITE);
    gfx->printf("%s  -->  %s", f->origin, f->dest);
  } else {
    gfx->setTextColor(COL_GRAY);
    gfx->print("General / Route Unavailable");
  }

  if (strlen(f->airline) > 0) {
    gfx->setTextColor(COL_GRAY);
    gfx->printf("  (%s)", f->airline);
  }

  gfx->setCursor(8, sheetY + 44);
  gfx->setTextColor(COL_GRAY); gfx->print("ALT: ");
  gfx->setTextColor(COL_WHITE);
  if (f->on_ground) {
    gfx->print("ON GROUND     ");
  } else {
    gfx->printf("%.0fft (%.0fm)   ", OpenSkyClient::metersToFeet(f->alt_m), f->alt_m);
  }

  gfx->setTextColor(COL_GRAY); gfx->print("SPD: ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%.0fkn", OpenSkyClient::msToKnots(f->vel_ms));

  gfx->setCursor(8, sheetY + 60);
  gfx->setTextColor(COL_GRAY); gfx->print("DIST: ");
  gfx->setTextColor(COL_YELLOW);
  gfx->printf("%.1fkm (%s)   ", f->dist_km, OpenSkyClient::getCompassDir(f->bearing));

  gfx->setTextColor(COL_GRAY); gfx->print("HDG: ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%.0f deg", f->track);

  if (strlen(f->aircraft) > 0) {
    gfx->setTextColor(COL_CYAN);
    gfx->printf("   TYPE: %s", f->aircraft);
  }

  gfx->setTextColor(COL_DIM_GRAY);
  gfx->setCursor(210, sheetY + 74);
  gfx->print("[Tap to close]");
}

void FlightListView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, 16, COL_HEADER_BG);
  gfx->drawFastHLine(0, CONTENT_Y + 15, SCREEN_W, COL_GRAY);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(4,   CONTENT_Y + 4); gfx->print("CALLSIGN");
  gfx->setCursor(70,  CONTENT_Y + 4); gfx->print("ROUTE");
  gfx->setCursor(148, CONTENT_Y + 4); gfx->print("ALTITUDE");
  gfx->setCursor(210, CONTENT_Y + 4); gfx->print("SPEED");
  gfx->setCursor(266, CONTENT_Y + 4); gfx->print("DISTANCE");

  int count = OpenSkyClient::getCount();
  if (count == 0) {
    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(80, CONTENT_Y + 80);
    gfx->print("No aircraft currently in range.");
    return;
  }

  int rows = min(count, (int)LIST_VISIBLE_ROWS);
  for (int i = 0; i < rows; i++) {
    drawRow(i, (i == selectedFlightIdx));
  }

  if (selectedFlightIdx >= 0 && selectedFlightIdx < count) {
    drawDetail(selectedFlightIdx);
  }
}

void FlightListView::handleTouch(int tx, int ty) {
  int count = OpenSkyClient::getCount();
  if (count == 0) return;

  if (selectedFlightIdx >= 0) {
    clearDetail();
    draw();
    return;
  }

  int relY = ty - (CONTENT_Y + 16);
  if (relY >= 0) {
    int row = relY / LIST_ROW_HEIGHT;
    if (row < count && row < (int)LIST_VISIBLE_ROWS) {
      selectedFlightIdx = row;
      draw();
    }
  }
}
