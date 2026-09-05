#include "DisplayEngine.h"
#include <SPI.h>

Arduino_DataBus *DisplayEngine::bus = nullptr;
Arduino_GFX     *DisplayEngine::gfx = nullptr;
XPT2046_Touchscreen DisplayEngine::ts(TOUCH_CS, TOUCH_IRQ);

static unsigned long lastTouchMs = 0;
static const unsigned long TOUCH_DEBOUNCE_MS = 250;

// Subclass ST7789 to force SPI_MODE0 instead of the library default SPI_MODE3 on ESP32
class CYD_ST7789 : public Arduino_ST7789 {
public:
  CYD_ST7789(Arduino_DataBus *bus, int8_t rst = GFX_NOT_DEFINED, uint8_t r = 0, bool ips = false,
             int16_t w = 240, int16_t h = 320,
             uint8_t col_offset1 = 0, uint8_t row_offset1 = 0,
             uint8_t col_offset2 = 0, uint8_t row_offset2 = 0)
    : Arduino_ST7789(bus, rst, r, ips, w, h, col_offset1, row_offset1, col_offset2, row_offset2) {}

  bool begin(int32_t speed = 20000000) override {
    _override_datamode = SPI_MODE0;
    bool ok = Arduino_TFT::begin(speed);
    invertDisplay(false);
    return ok;
  }
};

void DisplayEngine::begin() {
  Serial.println("[DisplayEngine] Configuring hardware GPIO pins...");

  // 1. Release all SPI chip selects before configuring SPI
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);

  // 2. Backlight pin setup - Turn on backlight HIGH
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // 3. Status LED & BOOT
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(BOARD_BOOT_PIN, INPUT_PULLUP);

  // 4. Initialize Hardware SPI bus at 20MHz on shared bus pins
  bus = new Arduino_HWSPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO, &SPI, true);

  // 5. Initialize ST7789 display (240x320, Rotation 3 = Landscape 320x240)
  Serial.println("[DisplayEngine] Initializing ST7789 with SPI_MODE0 at 20MHz...");
  gfx = new CYD_ST7789(bus, TFT_RST, 3 /* rotation */, false /* IPS */);
  if (!gfx->begin(20000000)) {
    Serial.println("[DisplayEngine] gfx->begin() returned false!");
  } else {
    Serial.println("[DisplayEngine] Display initialized successfully.");
  }

  // Send wake sequence in case board has alternative controller
  bus->beginWrite();
  bus->writeCommand(0x11); // SLPOUT
  delay(120);
  bus->writeCommand(0x29); // DISPON
  bus->endWrite();

  // Test fill screen
  gfx->fillScreen(COL_BG_DARK);

  // 6. Initialize XPT2046 Touch on shared SPI bus (reusing the same SPI instance)
  Serial.println("[DisplayEngine] Initializing XPT2046 Touch on shared SPI...");
  ts.begin(SPI);
  ts.setRotation(3);

  Serial.println("[DisplayEngine] Hardware setup complete.");
}

Arduino_GFX *DisplayEngine::getGfx() {
  return gfx;
}

bool DisplayEngine::readTouch(int &touchX, int &touchY) {
  if (!ts.touched()) return false;

  unsigned long now = millis();
  if (now - lastTouchMs < TOUCH_DEBOUNCE_MS) return false;

  TS_Point p = ts.getPoint();
  lastTouchMs = now;

  // Calibrate & map to screen resolution
  int tx = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_W);
  int ty = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_H);

  tx = constrain(tx, 0, SCREEN_W - 1);
  ty = constrain(ty, 0, SCREEN_H - 1);

  touchX = tx;
  touchY = ty;
  return true;
}

void DisplayEngine::drawHeader(const char *city, const char *timeStr, int flightCount, bool wifiOk) {
  if (!gfx) return;
  gfx->fillRect(0, 0, SCREEN_W, HEADER_H, COL_HEADER_BG);
  gfx->drawFastHLine(0, HEADER_H - 1, SCREEN_W, COL_GRAY);

  // City Name (Left) - clamp to max 16 chars to prevent overlap with clock
  char cBuf[20];
  snprintf(cBuf, sizeof(cBuf), "%.16s", city ? city : "EARTH");
  gfx->setTextColor(COL_CYAN);
  gfx->setTextSize(1);
  gfx->setCursor(6, 6);
  gfx->print(cBuf);

  // Center Time (Local with AM/PM)
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(110, 6);
  gfx->printf("%s", timeStr ? timeStr : "--:--:--");

  // Right status: flight count
  gfx->setCursor(186, 6);
  if (flightCount >= 0) {
    gfx->setTextColor(COL_GREEN);
    gfx->printf("AC:%2d", flightCount);
  } else {
    gfx->setTextColor(COL_YELLOW);
    gfx->print("AC:--");
  }

  // WiFi icon blip
  gfx->setCursor(226, 6);
  gfx->setTextColor(wifiOk ? COL_GREEN : COL_RED);
  gfx->print(wifiOk ? "[WF]" : "[NC]");

  // Interactive Setup button [SET]
  gfx->fillRoundRect(268, 2, 48, 17, 3, 0x0A54);
  gfx->drawRoundRect(268, 2, 48, 17, 3, COL_CYAN);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(278, 6);
  gfx->print("SET");
}

void DisplayEngine::drawFooter(AppMode activeMode) {
  if (!gfx) return;
  gfx->fillRect(0, SCREEN_H - FOOTER_H, SCREEN_W, FOOTER_H, COL_FOOTER_BG);
  gfx->drawFastHLine(0, SCREEN_H - FOOTER_H, SCREEN_W, COL_GRAY);

  const char *tabs[] = { "RADAR", "LIST", "WX", "ISS", "SPACEX", "CITY" };
  const int TAB_COUNT = 6;
  const int tabW = SCREEN_W / TAB_COUNT; // 53px each

  for (int i = 0; i < TAB_COUNT; i++) {
    int x = i * tabW;
    int w = (i == TAB_COUNT - 1) ? (SCREEN_W - x) : tabW;
    bool isActive = (i == (int)activeMode);

    if (isActive) {
      gfx->fillRect(x + 1, SCREEN_H - FOOTER_H + 2, w - 2, FOOTER_H - 4, COL_CYAN);
      gfx->setTextColor(COL_BLACK);
    } else {
      gfx->fillRect(x + 1, SCREEN_H - FOOTER_H + 2, w - 2, FOOTER_H - 4, COL_FOOTER_BG);
      gfx->setTextColor(COL_BTN_TEXT);
    }

    gfx->setTextSize(1);
    int textX = x + (w - (int)strlen(tabs[i]) * 6) / 2;
    gfx->setCursor(textX, SCREEN_H - FOOTER_H + 7);
    gfx->print(tabs[i]);

    if (i < TAB_COUNT - 1) {
      gfx->drawFastVLine(x + w, SCREEN_H - FOOTER_H + 3, FOOTER_H - 6, COL_DIM_GRAY);
    }
  }
}

void DisplayEngine::showStatus(const char *msg, uint16_t color) {
  if (!gfx) return;
  int py = SCREEN_H - FOOTER_H - 18;
  gfx->fillRect(10, py, SCREEN_W - 20, 16, COL_BLACK);
  gfx->drawRect(10, py, SCREEN_W - 20, 16, color);
  gfx->setTextColor(color);
  gfx->setTextSize(1);
  gfx->setCursor(18, py + 4);
  gfx->print(msg);
}

void DisplayEngine::drawWeatherIcon(int x, int y, int code, uint16_t color) {
  if (!gfx) return;
  if (code == 0) {
    gfx->fillCircle(x + 16, y + 16, 8, COL_YELLOW);
    for (int a = 0; a < 360; a += 45) {
      float rad = a * 0.0174533f;
      int x1 = x + 16 + (int)(11.0f * cosf(rad));
      int y1 = y + 16 + (int)(11.0f * sinf(rad));
      int x2 = x + 16 + (int)(14.0f * cosf(rad));
      int y2 = y + 16 + (int)(14.0f * sinf(rad));
      gfx->drawLine(x1, y1, x2, y2, COL_YELLOW);
    }
  }
  else if (code >= 1 && code <= 3) {
    gfx->fillCircle(x + 12, y + 18, 6, COL_GRAY);
    gfx->fillCircle(x + 20, y + 16, 8, COL_WHITE);
    gfx->fillCircle(x + 26, y + 19, 5, COL_GRAY);
    gfx->fillRect(x + 10, y + 20, 18, 6, COL_WHITE);
  }
  else if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) {
    gfx->fillCircle(x + 12, y + 14, 6, COL_GRAY);
    gfx->fillCircle(x + 20, y + 12, 8, COL_WHITE);
    gfx->fillRect(x + 10, y + 16, 18, 6, COL_WHITE);
    for (int i = 0; i < 3; i++) {
      gfx->drawLine(x + 12 + i * 6, y + 24, x + 10 + i * 6, y + 29, COL_CYAN);
    }
  }
  else if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) {
    gfx->fillCircle(x + 12, y + 14, 6, COL_GRAY);
    gfx->fillCircle(x + 20, y + 12, 8, COL_WHITE);
    gfx->fillRect(x + 10, y + 16, 18, 6, COL_WHITE);
    gfx->drawChar(x + 11, y + 23, '*', COL_WHITE, COL_BLACK);
    gfx->drawChar(x + 21, y + 23, '*', COL_WHITE, COL_BLACK);
  }
  else {
    gfx->fillCircle(x + 12, y + 14, 6, COL_GRAY);
    gfx->fillCircle(x + 20, y + 12, 8, COL_WHITE);
    gfx->fillRect(x + 10, y + 16, 18, 6, COL_WHITE);
    gfx->drawLine(x + 18, y + 22, x + 15, y + 27, COL_YELLOW);
    gfx->drawLine(x + 15, y + 27, x + 20, y + 27, COL_YELLOW);
    gfx->drawLine(x + 20, y + 27, x + 16, y + 32, COL_YELLOW);
  }
}

void DisplayEngine::drawMiniCompass(int cx, int cy, int r, float bearing, uint16_t color) {
  if (!gfx) return;
  gfx->drawCircle(cx, cy, r, COL_DIM_GRAY);
  gfx->drawPixel(cx, cy - r, COL_WHITE);
  gfx->drawPixel(cx + r, cy, COL_DIM_GRAY);
  gfx->drawPixel(cx - r, cy, COL_DIM_GRAY);
  gfx->drawPixel(cx, cy + r, COL_DIM_GRAY);

  float rad = (bearing - 90.0f) * 0.0174533f;
  int tipX = cx + (int)((r - 2) * cosf(rad));
  int tipY = cy + (int)((r - 2) * sinf(rad));
  gfx->drawLine(cx, cy, tipX, tipY, color);
  gfx->fillCircle(tipX, tipY, 2, color);
}

void DisplayEngine::setBrightness(uint8_t duty) {
  analogWrite(TFT_BL, duty);
}
