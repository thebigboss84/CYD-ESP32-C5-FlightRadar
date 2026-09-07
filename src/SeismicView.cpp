#include "SeismicView.h"
#include "DisplayEngine.h"
#include "SeismicClient.h"
#include "GpsManager.h"
#include "ConfigPortal.h"
#include <math.h>

void SeismicView::draw() {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  // Deep aerospace dark canvas
  gfx->fillRect(0, CONTENT_Y, SCREEN_W, CONTENT_H, COL_BG_DARK);

  const SeismicRecord &latest = SeismicClient::getLatest();
  bool alert = SeismicClient::hasActiveAlert();

  // 1. Primary Tremor Card (y: CONTENT_Y + 2 .. CONTENT_Y + 68)
  int cardY = CONTENT_Y + 2;
  int cardH = 66;
  gfx->fillRoundRect(4, cardY, SCREEN_W - 8, cardH, 4, alert ? 0x3000 : 0x0842);
  gfx->drawRoundRect(4, cardY, SCREEN_W - 8, cardH, 4, alert ? 0xF800 : 0x0269);

  // Left Magnitude Badge
  uint16_t magCol = COL_YELLOW;
  const char *magDesc = "MINOR";
  if (latest.mag >= 4.0f) {
    magCol = 0xF800; // Red
    magDesc = "STRONG";
  } else if (latest.mag >= 2.5f) {
    magCol = 0xFD20; // Amber / Orange
    magDesc = "MODERATE";
  } else if (latest.mag >= 2.0f) {
    magCol = 0x07E0; // Emerald Green
    magDesc = "NOTICEABLE";
  }

  // Magnitude square box
  gfx->fillRoundRect(8, cardY + 5, 52, 56, 3, 0x0000);
  gfx->drawRoundRect(8, cardY + 5, 52, 56, 3, magCol);

  gfx->setTextSize(1);
  gfx->setTextColor(magCol);
  gfx->setCursor(14, cardY + 9);
  gfx->print("RICHTER");

  gfx->setTextSize(2);
  gfx->setCursor(12, cardY + 22);
  if (latest.valid) {
    gfx->printf("M%.1f", latest.mag);
  } else {
    gfx->print("M--");
  }

  gfx->setTextSize(1);
  gfx->setCursor(11, cardY + 44);
  gfx->print(magDesc);

  // Middle & Right Details
  gfx->setTextSize(1);
  gfx->setTextColor(alert ? 0xF800 : COL_YELLOW);
  gfx->setCursor(66, cardY + 6);
  if (alert) {
    gfx->print("! ACTIVE SEISMIC WARNING (< 200km)");
  } else {
    gfx->print("USGS SEISMIC OBSERVATORY (< 200km)");
  }

  if (latest.valid) {
    // Epicenter place
    gfx->setTextColor(COL_WHITE);
    gfx->setCursor(66, cardY + 18);
    // Truncate place name if too long for 240px
    char placeBuf[36];
    strncpy(placeBuf, latest.place, sizeof(placeBuf) - 1);
    placeBuf[sizeof(placeBuf) - 1] = '\0';
    gfx->print(placeBuf);

    // Distance & Bearing
    gfx->setTextColor(COL_CYAN);
    gfx->setCursor(66, cardY + 30);
    gfx->printf("Dist: %.1f km (%.0f deg) | Depth: %.1f km",
               latest.dist_km, latest.bearing, latest.depth_km);

    // Age / Elapsed time
    gfx->setTextColor(latest.age_min <= 90 ? 0xFFE0 : COL_GRAY);
    gfx->setCursor(66, cardY + 42);
    gfx->printf("Occurred: %d min ago (Epoch: %s)",
               latest.age_min, latest.is_alert ? "ALERT" : "Normal");

    // Wave propagation status
    gfx->setTextColor(0x8410);
    gfx->setCursor(66, cardY + 54);
    gfx->print("P-Wave ~6km/s, S-Wave ~3.5km/s: Passed");
  } else {
    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(66, cardY + 24);
    gfx->print("No recent tremors reported by USGS.");
    gfx->setCursor(66, cardY + 40);
    gfx->print("Monitoring 200 km radius...");
  }

  // 2. Simulated Seismogram Waveform Trace (y: CONTENT_Y + 70 .. CONTENT_Y + 116, height 46px)
  drawSeismogram(4, CONTENT_Y + 70, SCREEN_W - 8, 44, latest.valid ? latest.mag : 1.0f);

  // 3. Recent Regional Tremors Roster (y: CONTENT_Y + 116 .. CONTENT_Y + 192)
  int listY = CONTENT_Y + 118;
  gfx->setTextSize(1);
  gfx->setTextColor(COL_YELLOW);
  gfx->setCursor(6, listY);
  gfx->print("RECENT REGIONAL TREMORS (< 200km):");

  int count = SeismicClient::getCount();
  int rowY = listY + 11;

  for (int i = 0; i < count && i < 4; i++) {
    const SeismicRecord *rec = SeismicClient::getRecord(i);
    if (!rec || !rec->valid) continue;

    // Alternating faint row background
    if (i % 2 == 1) {
      gfx->fillRect(4, rowY - 1, SCREEN_W - 8, 14, 0x0842);
    }

    // Magnitude badge
    uint16_t rowCol = (rec->mag >= 3.0f) ? 0xF800 : (rec->mag >= 2.2f ? 0xFD20 : 0x07E0);
    gfx->setTextColor(rowCol);
    gfx->setCursor(6, rowY + 2);
    gfx->printf("M%.1f", rec->mag);

    // Distance
    gfx->setTextColor(COL_CYAN);
    gfx->setCursor(34, rowY + 2);
    gfx->printf("%3.0fkm", rec->dist_km);

    // Age
    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(76, rowY + 2);
    gfx->printf("%3dm", rec->age_min);

    // Location
    gfx->setTextColor(COL_WHITE);
    gfx->setCursor(106, rowY + 2);
    char rowPlace[32];
    strncpy(rowPlace, rec->place, sizeof(rowPlace) - 1);
    rowPlace[sizeof(rowPlace) - 1] = '\0';
    gfx->print(rowPlace);

    rowY += 15;
  }
}

void SeismicView::drawSeismogram(int x, int y, int w, int h, float mag) {
  Arduino_GFX *gfx = DisplayEngine::getGfx();
  if (!gfx) return;

  // Background box for seismograph
  gfx->fillRect(x, y, w, h, 0x0000);
  gfx->drawRect(x, y, w, h, 0x0269);

  // Center horizontal baseline
  int midY = y + h / 2;
  gfx->drawFastHLine(x + 1, midY, w - 2, 0x0185);

  // Header tag
  gfx->setTextSize(1);
  gfx->setTextColor(0x04F5);
  gfx->setCursor(x + 4, y + 3);
  gfx->print("SEISMOGRAM Z-TRACE");

  // Time grid vertical ticks every 30px
  for (int gx = x + 30; gx < x + w; gx += 40) {
    gfx->drawFastVLine(gx, y + 1, h - 2, 0x0103);
  }

  // Wave amplitude scaling based on magnitude
  float ampScale = fminf(fmaxf((mag - 1.0f) * 4.0f, 2.0f), (float)(h / 2 - 3));

  int prevX = x + 1;
  int prevY = midY;

  // Trace waveform across width
  for (int px = x + 1; px < x + w - 1; px++) {
    float normX = (float)(px - x) / (float)w; // 0.0 to 1.0
    float wave = 0.0f;

    // Ambient microseismic background noise
    wave += sinf(normX * 90.0f) * 1.0f + sinf(normX * 210.0f) * 0.8f;

    // P-Wave arrival packet (around 30% width)
    if (normX >= 0.25f && normX <= 0.45f) {
      float pEnv = sinf((normX - 0.25f) / 0.20f * (float)M_PI);
      wave += sinf((normX - 0.25f) * 120.0f) * (ampScale * 0.45f) * pEnv;
    }

    // S-Wave and surface coda wave packet (around 45% to 85% width)
    if (normX >= 0.45f && normX <= 0.88f) {
      float sEnv = sinf((normX - 0.45f) / 0.43f * (float)M_PI);
      wave += sinf((normX - 0.45f) * 70.0f) * ampScale * sEnv;
      wave += sinf((normX - 0.45f) * 160.0f) * (ampScale * 0.3f) * sEnv;
    }

    int curY = midY + (int)wave;
    if (curY < y + 1) curY = y + 1;
    if (curY > y + h - 2) curY = y + h - 2;

    uint16_t waveColor = (normX >= 0.45f && normX <= 0.85f && mag >= 2.5f) ? 0xFD20 : 0x07E0;
    gfx->drawLine(prevX, prevY, px, curY, waveColor);

    prevX = px;
    prevY = curY;
  }
}

bool SeismicView::handleTouch(int tx, int ty) {
  // Tap anywhere on seismic view to force refresh from USGS
  Serial.println("[SeismicView] Touch registered - refreshing USGS telemetry...");
  return true;
}
