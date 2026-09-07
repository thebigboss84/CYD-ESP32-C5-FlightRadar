#include "FlightListView.h"
#include "DisplayEngine.h"
#include "OpenSkyClient.h"

int FlightListView::selectedFlightIdx = -1;
int FlightListView::currentPage = 0;

static const int ROWS_PER_PAGE = 6;
static const int ROW_H = 22;

void FlightListView::clearDetail() {
  selectedFlightIdx = -1;
}

void FlightListView::drawRow(int flightIdx, int displayRow, bool isSelected) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const FlightRecord *f = OpenSkyClient::getFlight(flightIdx);
  if (!f) return;

  int y = CONTENT_Y + 15 + (displayRow * ROW_H);
  uint16_t rowBg = isSelected ? 0x0842 : ((displayRow % 2 == 0) ? 0x0006 : COL_BG_DARK);

  gfx->fillRect(0, y, SCREEN_W, ROW_H, rowBg);
  if (isSelected) {
    gfx->drawFastHLine(0, y, SCREEN_W, COL_CYAN);
    gfx->drawFastHLine(0, y + ROW_H - 1, SCREEN_W, COL_CYAN);
  }

  uint16_t textColor = COL_AC_HIGH;
  if (f->on_ground) textColor = COL_AC_GND;
  else if (!isnan(f->alt_m) && f->alt_m < 3048.0f) textColor = COL_AC_LOW;

  gfx->setTextSize(1);

  // 1. Callsign (X: 4..66)
  gfx->setTextColor(textColor);
  gfx->setCursor(4, y + 6);
  gfx->print(f->callsign);

  // 2. Route / Cities (X: 68..152)
  gfx->setCursor(68, y + 6);
  if (strlen(f->origin_city) > 0 && strlen(f->dest_city) > 0) {
    gfx->setTextColor(COL_WHITE);
    char routeBuf[20];
    snprintf(routeBuf, sizeof(routeBuf), "%.6s>%.6s", f->origin_city, f->dest_city);
    gfx->print(routeBuf);
  } else if (strlen(f->origin_code) > 0 && strlen(f->dest_code) > 0) {
    gfx->setTextColor(COL_CYAN);
    gfx->printf("%-3s > %-3s", f->origin_code, f->dest_code);
  } else {
    gfx->setTextColor(COL_DIM_GRAY);
    gfx->print("--- > ---");
  }

  // 3. Altitude (X: 154..206)
  gfx->setCursor(154, y + 6);
  gfx->setTextColor(textColor);
  if (f->on_ground) {
    gfx->print("   GND");
  } else if (isnan(f->alt_m)) {
    gfx->print("  ----");
  } else {
    gfx->printf("%5.0fft", OpenSkyClient::metersToFeet(f->alt_m));
  }

  // 4. Speed (X: 208..254)
  gfx->setCursor(210, y + 6);
  gfx->setTextColor(COL_GRAY);
  if (f->vel_ms > 1.0f) {
    gfx->printf("%3.0fkn", OpenSkyClient::msToKnots(f->vel_ms));
  } else {
    gfx->print("  0kn");
  }

  // 5. Distance (X: 256..318)
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(262, y + 6);
  gfx->printf("%4.0fkm", f->dist_km);
}

void FlightListView::drawDetail(int index) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const FlightRecord *f = OpenSkyClient::getFlight(index);
  if (!f) return;

  // Auto-fetch full route if not fetched yet
  if (!f->route_fetched) {
    OpenSkyClient::fetchRoute(index);
  }

  int sheetY = CONTENT_Y + 16;
  int sheetH = CONTENT_H - 24;

  gfx->fillRoundRect(4, sheetY, SCREEN_W - 8, sheetH, 6, 0x0842);
  gfx->drawRoundRect(4, sheetY, SCREEN_W - 8, sheetH, 6, COL_CYAN);

  // Title Banner
  gfx->fillRoundRect(6, sheetY + 2, SCREEN_W - 12, 18, 4, 0x0185);
  gfx->setTextSize(1);
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(12, sheetY + 6);
  gfx->printf("FLIGHT: %s", f->callsign);

  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(140, sheetY + 6);
  gfx->printf("ICAO: %s", f->icao);

  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(230, sheetY + 6);
  gfx->printf("%.10s", f->country);

  // Route Origin
  gfx->setCursor(10, sheetY + 26);
  gfx->setTextColor(COL_GRAY);  gfx->print("FROM: ");
  gfx->setTextColor(COL_WHITE);
  if (strlen(f->origin_city) > 0) {
    gfx->printf("%s (%s) - %.18s", f->origin_city, f->origin_code, f->origin_name);
  } else if (strlen(f->origin_code) > 0) {
    gfx->printf("Airport %s", f->origin_code);
  } else {
    gfx->print("Unknown Origin");
  }

  // Route Destination
  gfx->setCursor(10, sheetY + 40);
  gfx->setTextColor(COL_GRAY);  gfx->print("TO:   ");
  gfx->setTextColor(COL_YELLOW);
  if (strlen(f->dest_city) > 0) {
    gfx->printf("%s (%s) - %.18s", f->dest_city, f->dest_code, f->dest_name);
  } else if (strlen(f->dest_code) > 0) {
    gfx->printf("Airport %s", f->dest_code);
  } else {
    gfx->print("Unknown Destination");
  }

  // Carrier & Aircraft
  gfx->setCursor(10, sheetY + 54);
  gfx->setTextColor(COL_GRAY); gfx->print("AIRL: ");
  gfx->setTextColor(COL_CYAN);
  gfx->printf("%.18s", strlen(f->airline) > 0 ? f->airline : "Commercial / Private");
  if (strlen(f->aircraft) > 0) {
    gfx->setTextColor(COL_GRAY); gfx->print(" | TYPE: ");
    gfx->setTextColor(COL_WHITE);
    gfx->printf("%s", f->aircraft);
  }

  // Flight Dynamics Line 1: Alt & Speed
  gfx->setCursor(10, sheetY + 68);
  gfx->setTextColor(COL_GRAY); gfx->print("ALT:  ");
  gfx->setTextColor(COL_WHITE);
  if (f->on_ground) gfx->print("ON GROUND      ");
  else gfx->printf("%.0fft (%.0fm)   ", OpenSkyClient::metersToFeet(f->alt_m), f->alt_m);

  gfx->setTextColor(COL_GRAY); gfx->print("SPD: ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%.0fkn (%.0f km/h)", OpenSkyClient::msToKnots(f->vel_ms), f->vel_ms * 3.6f);

  // Flight Dynamics Line 2: Distance & Track
  gfx->setCursor(10, sheetY + 82);
  gfx->setTextColor(COL_GRAY); gfx->print("DIST: ");
  gfx->setTextColor(COL_YELLOW);
  gfx->printf("%.1fkm (%s)   ", f->dist_km, OpenSkyClient::getCompassDir(f->bearing));

  gfx->setTextColor(COL_GRAY); gfx->print("HDG: ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%.0f deg", f->track);

  // Close prompt
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(SCREEN_W - 100, sheetY + sheetH - 12);
  gfx->print("[Tap to close]");
}

void FlightListView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  // Column Header Bar
  gfx->fillRect(0, CONTENT_Y, SCREEN_W, 14, COL_HEADER_BG);
  gfx->drawFastHLine(0, CONTENT_Y + 14, SCREEN_W, COL_GRAY);

  gfx->setTextSize(1);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(4,   CONTENT_Y + 3); gfx->print("CALLSIGN");
  gfx->setCursor(68,  CONTENT_Y + 3); gfx->print("ROUTE/CITY");
  gfx->setCursor(154, CONTENT_Y + 3); gfx->print("ALTITUDE");
  gfx->setCursor(210, CONTENT_Y + 3); gfx->print("SPEED");
  gfx->setCursor(262, CONTENT_Y + 3); gfx->print("DIST");

  int totalFlights = OpenSkyClient::getCount();
  if (totalFlights == 0) {
    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(80, CONTENT_Y + 80);
    gfx->print("No aircraft currently in range.");
    return;
  }

  int totalPages = (totalFlights + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
  if (currentPage >= totalPages) currentPage = totalPages - 1;
  if (currentPage < 0) currentPage = 0;

  int startIdx = currentPage * ROWS_PER_PAGE;
  int endIdx = min(startIdx + ROWS_PER_PAGE, totalFlights);

  for (int i = startIdx; i < endIdx; i++) {
    drawRow(i, i - startIdx, (i == selectedFlightIdx));
  }

  // Bottom Pagination Navigation Bar (Y: CONTENT_Y + 150)
  int navY = CONTENT_Y + 152;
  gfx->drawFastHLine(0, navY, SCREEN_W, COL_DIM_GRAY);

  // Prev Button
  gfx->fillRoundRect(6, navY + 3, 68, 18, 3, (currentPage > 0) ? 0x0842 : 0x0002);
  gfx->drawRoundRect(6, navY + 3, 68, 18, 3, (currentPage > 0) ? COL_CYAN : COL_DIM_GRAY);
  gfx->setTextColor((currentPage > 0) ? COL_WHITE : COL_DIM_GRAY);
  gfx->setCursor(14, navY + 8);
  gfx->print("< PREV");

  // Page Indicator
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(86, navY + 8);
  gfx->printf("PAGE %d/%d (%d-%d of %d)",
              currentPage + 1, totalPages, startIdx + 1, endIdx, totalFlights);

  // Next Button
  gfx->fillRoundRect(SCREEN_W - 74, navY + 3, 68, 18, 3, (currentPage < totalPages - 1) ? 0x0842 : 0x0002);
  gfx->drawRoundRect(SCREEN_W - 74, navY + 3, 68, 18, 3, (currentPage < totalPages - 1) ? COL_CYAN : COL_DIM_GRAY);
  gfx->setTextColor((currentPage < totalPages - 1) ? COL_WHITE : COL_DIM_GRAY);
  gfx->setCursor(SCREEN_W - 64, navY + 8);
  gfx->print("NEXT >");

  if (selectedFlightIdx >= 0 && selectedFlightIdx < totalFlights) {
    drawDetail(selectedFlightIdx);
  }
}

void FlightListView::handleTouch(int tx, int ty) {
  int totalFlights = OpenSkyClient::getCount();
  if (totalFlights == 0) return;

  if (selectedFlightIdx >= 0) {
    clearDetail();
    draw();
    return;
  }

  int totalPages = (totalFlights + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
  int navY = CONTENT_Y + 152;

  // 1. Check Prev / Next Button Touches
  if (ty >= navY) {
    // Prev button touch (x: 0..80)
    if (tx < 80) {
      if (currentPage > 0) {
        currentPage--;
        draw();
      }
      return;
    }
    // Next button touch (x: SCREEN_W - 80..SCREEN_W)
    else if (tx > SCREEN_W - 80) {
      if (currentPage < totalPages - 1) {
        currentPage++;
        draw();
      }
      return;
    }
    // Center area tap cycles page
    else {
      currentPage = (currentPage + 1) % totalPages;
      draw();
      return;
    }
  }

  // 2. Check Row Touches
  int relY = ty - (CONTENT_Y + 15);
  if (relY >= 0 && relY < ROWS_PER_PAGE * ROW_H) {
    int row = relY / ROW_H;
    int flightIdx = (currentPage * ROWS_PER_PAGE) + row;
    if (flightIdx < totalFlights) {
      selectedFlightIdx = flightIdx;
      draw();
    }
  }
}
