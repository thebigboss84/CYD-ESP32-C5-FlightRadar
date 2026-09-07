#include "FlightRadarView.h"
#include "DisplayEngine.h"
#include "OpenSkyClient.h"
#include "GpsManager.h"
#include <math.h>

float FlightRadarView::sweepAngle = 0.0f;

void FlightRadarView::draw(float radiusKm) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  // 1. Deep aerospace pitch-black canvas
  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, 0x0000);

  const int cx = 160;
  const int cy = 119;
  const int r  = 90;

  // 2. Dual Outer Bezel Ring (Tactical Cyan / Teal)
  gfx->drawCircle(cx, cy, r,     0x04F5); // Bright tactical cyan bezel
  gfx->drawCircle(cx, cy, r + 1, 0x0269); // Darker shadow ring

  // Compass ticks around outer rim (every 45 deg)
  for (int a = 0; a < 360; a += 45) {
    float rad = (float)a * (float)M_PI / 180.0f;
    int x1 = cx + (int)(sinf(rad) * (r - 1));
    int y1 = cy - (int)(cosf(rad) * (r - 1));
    int x2 = cx + (int)(sinf(rad) * (r - 4));
    int y2 = cy - (int)(cosf(rad) * (r - 4));
    gfx->drawLine(x1, y1, x2, y2, 0x0357);
  }

  // Cardinal direction markers
  gfx->setTextSize(1);
  gfx->setTextColor(0x07FF); // Bright cyan North
  gfx->setCursor(cx - 3, cy - r - 9); gfx->print("N");

  gfx->setTextColor(0x0357); // Muted teal for S, E, W
  gfx->setCursor(cx + r + 4, cy - 3);  gfx->print("E");
  gfx->setCursor(cx - 3, cy + r + 2);  gfx->print("S");
  gfx->setCursor(cx - r - 9, cy - 3); gfx->print("W");

  // 3. Subtle Inner Range Rings (50% and 100% range)
  gfx->drawCircle(cx, cy, r / 2, 0x0185); // Faint dark cyan 50% ring

  // Faint gapped crosshairs
  gfx->drawFastHLine(cx - r + 6, cy, (r / 2) - 10, 0x0144);
  gfx->drawFastHLine(cx + 4,     cy, (r / 2) - 10, 0x0144);
  gfx->drawFastVLine(cx, cy - r + 6, (r / 2) - 10, 0x0144);
  gfx->drawFastVLine(cx, cy + 4,     (r / 2) - 10, 0x0144);

  // Range distance tags (placed at top-right diagonal)
  char rBuf[16];
  gfx->setTextColor(0x02CF);
  snprintf(rBuf, sizeof(rBuf), "%.0fk", radiusKm * 0.5f);
  gfx->setCursor(cx + (r / 2) - 10, cy - (r / 2) + 2);
  gfx->print(rBuf);

  snprintf(rBuf, sizeof(rBuf), "%.0fkm", radiusKm);
  gfx->setCursor(cx + r - 32, cy - (r * 7 / 10));
  gfx->print(rBuf);

  // Center Home / GPS Location Bullseye
  gfx->drawCircle(cx, cy, 3, 0x07E0);
  gfx->drawPixel(cx, cy, COL_WHITE);

  // 4. Corner Avionics Badges
  int count = OpenSkyClient::getCount();

  // Top-Left Badge: Range & Targets & GPS status
  gfx->fillRoundRect(4, CONTENT_Y + 4, 62, 34, 4, 0x0842);
  gfx->drawRoundRect(4, CONTENT_Y + 4, 62, 34, 4, 0x0269);

  gfx->setTextColor(0x8410); gfx->setCursor(8, CONTENT_Y + 7);  gfx->print("RNG:");
  gfx->setTextColor(COL_CYAN); gfx->printf("%.0fk", radiusKm);

  gfx->setTextColor(0x8410); gfx->setCursor(8, CONTENT_Y + 17); gfx->print("TGT:");
  gfx->setTextColor(count > 0 ? 0x07E0 : COL_YELLOW); gfx->printf("%d", count);

  gfx->setCursor(8, CONTENT_Y + 27);
  if (GpsManager::hasFix()) {
    gfx->setTextColor(0x07E0);
    gfx->printf("GPS:%dsat", GpsManager::getSatellites());
  } else {
    gfx->setTextColor(0x8410);
    gfx->print("GPS:N/A");
  }

  // Top-Right Badge: Closest Target
  gfx->fillRoundRect(SCREEN_W - 64, CONTENT_Y + 4, 60, 34, 4, 0x0842);
  gfx->drawRoundRect(SCREEN_W - 64, CONTENT_Y + 4, 60, 34, 4, 0x0269);
  gfx->setTextColor(0x8410);
  gfx->setCursor(SCREEN_W - 60, CONTENT_Y + 7);
  gfx->print("CLOSEST");
  if (count > 0) {
    const FlightRecord *nr = OpenSkyClient::getFlight(0);
    if (nr) {
      gfx->setTextColor(COL_YELLOW);
      gfx->setCursor(SCREEN_W - 60, CONTENT_Y + 18);
      char nrBuf[12];
      snprintf(nrBuf, sizeof(nrBuf), "%.0fk", nr->dist_km);
      gfx->print(nrBuf);

      gfx->setTextColor(COL_CYAN);
      gfx->setCursor(SCREEN_W - 60, CONTENT_Y + 28);
      char csBuf[10];
      snprintf(csBuf, sizeof(csBuf), "%.7s", nr->callsign);
      gfx->print(csBuf);
    }
  } else {
    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(SCREEN_W - 60, CONTENT_Y + 18);
    gfx->print("NONE");
  }

  // Bottom Altitude Color Keys
  gfx->setTextColor(0x07FF); gfx->setCursor(6, CONTENT_Y + CONTENT_H - 18); gfx->print("^ >20k");
  gfx->setTextColor(0x07E0); gfx->setCursor(6, CONTENT_Y + CONTENT_H - 9);  gfx->print("- 10-20k");

  gfx->setTextColor(0xFFE0); gfx->setCursor(SCREEN_W - 52, CONTENT_Y + CONTENT_H - 18); gfx->print("v <10k");
  gfx->setTextColor(0x632C); gfx->setCursor(SCREEN_W - 52, CONTENT_Y + CONTENT_H - 9);  gfx->print("_ GND");

  // 5. Draw Aircraft Targets (Crisp, Directional Chevrons)
  float scale = (float)r / radiusKm;
  int labelsDrawn = 0;

  for (int i = 0; i < count; i++) {
    const FlightRecord *f = OpenSkyClient::getFlight(i);
    if (!f) continue;

    float bngRad = f->bearing * (float)M_PI / 180.0f;
    int sx = cx + (int)(sinf(bngRad) * f->dist_km * scale);
    int sy = cy - (int)(cosf(bngRad) * f->dist_km * scale);

    // Clip strictly within radar scope circle
    int dx = sx - cx;
    int dy = sy - cy;
    if ((dx * dx + dy * dy) > (r - 2) * (r - 2)) continue;

    // Altitude Color
    uint16_t col;
    if (f->on_ground) col = 0x632C;                     // Muted gray
    else if (!isnan(f->alt_m) && f->alt_m > 6096.0f) col = 0x07FF; // Cyan (> 20,000 ft)
    else if (!isnan(f->alt_m) && f->alt_m > 3048.0f) col = 0x07E0; // Emerald Green (10k - 20k ft)
    else col = 0xFFE0;                                  // Amber Yellow (< 10,000 ft)

    if (f->on_ground) {
      // Ground target: small 3x3 square
      gfx->fillRect(sx - 1, sy - 1, 3, 3, col);
    } else {
      // Airborne target: high-tech directional chevron
      float trkRad = f->track * (float)M_PI / 180.0f;
      float sinT = sinf(trkRad);
      float cosT = cosf(trkRad);

      // Core illuminated blip
      gfx->fillCircle(sx, sy, 2, col);
      gfx->drawPixel(sx, sy, COL_WHITE);

      // Velocity / heading vector line (7 pixels)
      int hx = sx + (int)(sinT * 8.0f);
      int hy = sy - (int)(cosT * 8.0f);
      gfx->drawLine(sx, sy, hx, hy, col);

      // Mini wings crossbar (4 pixels)
      int wx = sx + (int)(sinT * 3.0f);
      int wy = sy - (int)(cosT * 3.0f);
      int w1x = wx + (int)(-cosT * 3.0f);
      int w1y = wy - (int)( sinT * 3.0f);
      int w2x = wx - (int)(-cosT * 3.0f);
      int w2y = wy + (int)( sinT * 3.0f);
      gfx->drawLine(w1x, w1y, w2x, w2y, col);
    }

    // Smart Callout Tags: ONLY label top 3 closest aircraft
    if (!f->on_ground && labelsDrawn < 3 && strlen(f->callsign) > 1) {
      labelsDrawn++;
      int tagLen = strlen(f->callsign);
      int tagW = tagLen * 6 + 4;
      int tagH = 9;

      int tx = sx + 8;
      int ty = sy - 4;
      if (tx + tagW > SCREEN_W - 68) tx = sx - tagW - 8;
      if (ty < CONTENT_Y + 40) ty = CONTENT_Y + 40;
      if (ty + tagH > CONTENT_Y + CONTENT_H - 22) ty = CONTENT_Y + CONTENT_H - 31;

      // Solid background card
      gfx->fillRect(tx - 1, ty - 1, tagW, tagH, 0x0000);
      gfx->drawRect(tx - 1, ty - 1, tagW, tagH, 0x0269);

      // Leader line
      gfx->drawLine(sx, sy, (tx > sx) ? (tx - 1) : (tx + tagW), ty + 4, 0x0269);

      // Callsign
      gfx->setTextColor(col);
      gfx->setCursor(tx + 1, ty);
      gfx->print(f->callsign);
    }
  }

  // Clean empty state
  if (count == 0) {
    gfx->fillRoundRect(cx - 58, cy - 8, 116, 18, 4, 0x0842);
    gfx->drawRoundRect(cx - 58, cy - 8, 116, 18, 4, 0x0269);
    gfx->setTextColor(COL_YELLOW);
    gfx->setCursor(cx - 50, cy - 3);
    gfx->print("AIRSPACE CLEAR");
  }
}

void FlightRadarView::updateSweep(float radiusKm) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  const int cx = 160;
  const int cy = 119;
  const int r  = 90;

  // 1. Erase previous sweep line (from r=4 to r-2)
  float prevRad = sweepAngle * (float)M_PI / 180.0f;
  int px1 = cx + (int)(sinf(prevRad) * 4.0f);
  int py1 = cy - (int)(cosf(prevRad) * 4.0f);
  int px2 = cx + (int)(sinf(prevRad) * (r - 2));
  int py2 = cy - (int)(cosf(prevRad) * (r - 2));
  gfx->drawLine(px1, py1, px2, py2, 0x0000);

  // Restore 50% ring pixel at previous angle
  int rMidX = cx + (int)(sinf(prevRad) * (r / 2));
  int rMidY = cy - (int)(cosf(prevRad) * (r / 2));
  gfx->drawPixel(rMidX, rMidY, 0x0185);

  // Advance angle gently (3 degrees per tick)
  sweepAngle += 3.0f;
  if (sweepAngle >= 360.0f) sweepAngle -= 360.0f;

  // 2. Draw new sweep line
  float rad = sweepAngle * (float)M_PI / 180.0f;
  int nx1 = cx + (int)(sinf(rad) * 4.0f);
  int ny1 = cy - (int)(cosf(rad) * 4.0f);
  int nx2 = cx + (int)(sinf(rad) * (r - 2));
  int ny2 = cy - (int)(cosf(rad) * (r - 2));
  gfx->drawLine(nx1, ny1, nx2, ny2, 0x02E5); // faint tactical green/cyan beam

  // Restore center bullseye
  gfx->drawCircle(cx, cy, 3, 0x07E0);
  gfx->drawPixel(cx, cy, COL_WHITE);

  // 3. Highlight any aircraft target that the beam just swept across
  int count = OpenSkyClient::getCount();
  float scale = (float)r / radiusKm;
  for (int i = 0; i < count; i++) {
    const FlightRecord *f = OpenSkyClient::getFlight(i);
    if (!f || f->on_ground) continue;

    float angleDiff = fabsf(f->bearing - sweepAngle);
    if (angleDiff > 180.0f) angleDiff = 360.0f - angleDiff;

    // Ping target if beam is within 3.5 degrees
    if (angleDiff <= 3.5f) {
      float bngRad = f->bearing * (float)M_PI / 180.0f;
      int sx = cx + (int)(sinf(bngRad) * f->dist_km * scale);
      int sy = cy - (int)(cosf(bngRad) * f->dist_km * scale);
      int dx = sx - cx;
      int dy = sy - cy;
      if ((dx * dx + dy * dy) <= (r - 2) * (r - 2)) {
        gfx->fillCircle(sx, sy, 3, COL_WHITE); // Phosphor ping
      }
    }
  }
}
