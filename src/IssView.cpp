#include "IssView.h"
#include "DisplayEngine.h"
#include "IssClient.h"
#include "OpenSkyClient.h"
#include <math.h>

void IssView::draw(const char *cityName) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  const IssRecord &iss = IssClient::getData();
  if (!iss.valid) {
    gfx->setTextColor(COL_GRAY);
    gfx->setTextSize(1);
    gfx->setCursor(80, CONTENT_Y + 80);
    gfx->print("Acquiring ISS orbital telemetry...");
    return;
  }

  gfx->setTextSize(1);
  gfx->setTextColor(COL_PURPLE);
  gfx->setCursor(10, CONTENT_Y + 8);
  gfx->print("INTERNATIONAL SPACE STATION (ISS)");

  gfx->drawFastHLine(10, CONTENT_Y + 20, SCREEN_W - 20, COL_DIM_GRAY);

  int boxY = CONTENT_Y + 28;
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(10, boxY); gfx->print("LATITUDE:  ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%7.2f deg %s\n", fabsf(iss.lat), (iss.lat >= 0) ? "N" : "S");

  gfx->setCursor(10, boxY + 16);
  gfx->setTextColor(COL_GRAY); gfx->print("LONGITUDE: ");
  gfx->setTextColor(COL_WHITE);
  gfx->printf("%7.2f deg %s\n", fabsf(iss.lon), (iss.lon >= 0) ? "E" : "W");

  gfx->setCursor(10, boxY + 32);
  gfx->setTextColor(COL_GRAY); gfx->print("ALTITUDE:  ");
  gfx->setTextColor(COL_CYAN);
  gfx->printf("%.1f km\n", iss.alt_km);

  gfx->setCursor(10, boxY + 48);
  gfx->setTextColor(COL_GRAY); gfx->print("VELOCITY:  ");
  gfx->setTextColor(COL_YELLOW);
  gfx->printf("%.0f km/h\n", iss.velocity_kmh);

  gfx->setCursor(10, boxY + 64);
  gfx->setTextColor(COL_GRAY); gfx->print("SUNLIGHT:  ");
  if (iss.daylight) {
    gfx->setTextColor(COL_YELLOW);
    gfx->print("DAYLIGHT");
  } else {
    gfx->setTextColor(COL_CYAN);
    gfx->print("ECLIPSED (SHADOW)");
  }

  gfx->drawFastHLine(10, boxY + 80, SCREEN_W - 20, COL_DIM_GRAY);

  gfx->setCursor(10, boxY + 88);
  gfx->setTextColor(COL_GRAY);
  gfx->printf("FROM %s:\n", cityName);

  gfx->setCursor(10, boxY + 104);
  gfx->setTextColor(COL_WHITE);
  gfx->printf("RANGE:   %.0f km\n", iss.dist_km);

  gfx->setCursor(10, boxY + 120);
  gfx->printf("BEARING: %.0f deg (%s)\n", iss.bearing, OpenSkyClient::getCompassDir(iss.bearing));

  if (iss.dist_km < 800.0f) {
    gfx->setTextColor(COL_GREEN);
    gfx->setCursor(10, boxY + 136);
    gfx->print("*** IN LOCAL VISIBLE SKY PASS ***");
  }

  int compX = 245;
  int compY = CONTENT_Y + 95;
  int compR = 48;

  gfx->drawCircle(compX, compY, compR, COL_GREEN);
  gfx->drawCircle(compX, compY, compR / 2, COL_DIM_GRAY);
  gfx->drawFastHLine(compX - compR, compY, compR * 2, COL_DIM_GRAY);
  gfx->drawFastVLine(compX, compY - compR, compR * 2, COL_DIM_GRAY);

  gfx->setTextColor(COL_GREEN);
  gfx->setCursor(compX - 2, compY - compR - 8); gfx->print("N");
  gfx->setCursor(compX + compR + 4, compY - 3);  gfx->print("E");
  gfx->setCursor(compX - 2, compY + compR + 2);  gfx->print("S");
  gfx->setCursor(compX - compR - 8, compY - 3);  gfx->print("W");

  float rad = (iss.bearing - 90.0f) * 0.0174533f;
  int issX = compX + (int)((compR - 4) * cosf(rad));
  int issY = compY + (int)((compR - 4) * sinf(rad));

  gfx->drawLine(compX, compY, issX, issY, COL_YELLOW);
  gfx->fillCircle(issX, issY, 4, COL_YELLOW);
  gfx->drawCircle(issX, issY, 6, COL_WHITE);

  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(compX - 10, compY + compR + 14);
  gfx->print("ISS SKY");
}
