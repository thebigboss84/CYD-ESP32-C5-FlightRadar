#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "AppConfig.h"
#include "DisplayEngine.h"
#include "ConfigPortal.h"
#include "OpenSkyClient.h"
#include "WeatherClient.h"
#include "IssClient.h"
#include "SpaceXClient.h"
#include "FlightRadarView.h"
#include "FlightListView.h"
#include "WeatherView.h"
#include "IssView.h"
#include "SpaceXView.h"

static AppMode currentMode = MODE_RADAR;

static unsigned long lastOpenSkyFetch = 0;
static unsigned long lastWeatherFetch = 0;
static unsigned long lastIssFetch     = 0;
static unsigned long lastSpaceXFetch  = 0;
static unsigned long lastClockTick    = 0;
static unsigned long lastSweepTick    = 0;
static unsigned long lastCountdownTick = 0;

static char timeString[24] = "12:00:00 AM";

static void updateClockString() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeString, sizeof(timeString), "%I:%M:%S %p", &timeinfo);
  }
}

static void redrawCurrentView() {
  updateClockString();
  DisplayEngine::drawHeader(ConfigPortal::getCityName(), timeString, OpenSkyClient::getCount(), WiFi.isConnected());

  switch (currentMode) {
    case MODE_RADAR:
      FlightRadarView::draw(ConfigPortal::getRadius());
      break;
    case MODE_FLIGHT_LIST:
      FlightListView::draw();
      break;
    case MODE_WEATHER:
      WeatherView::draw(ConfigPortal::getCityName());
      break;
    case MODE_ISS:
      IssView::draw(ConfigPortal::getCityName());
      break;
    case MODE_SPACEX:
      SpaceXView::draw();
      break;
    default:
      break;
  }

  DisplayEngine::drawFooter(currentMode);
}

static void fetchCityData() {
  DisplayEngine::showStatus("Acquiring airspace & satellite telemetry...", COL_YELLOW);
  configTzTime(ConfigPortal::getTimeZone(), "pool.ntp.org", "time.nist.gov");
  OpenSkyClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon(), ConfigPortal::getRadius());
  WeatherClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon());
  IssClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon());
  SpaceXClient::fetch();

  lastOpenSkyFetch = millis();
  lastWeatherFetch = millis();
  lastIssFetch     = millis();
  lastSpaceXFetch  = millis();

  redrawCurrentView();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==========================================");
  Serial.println("  CYD ESP32-C5 AirRadar & Aerospace Instrument");
  Serial.println("==========================================");

  pinMode(BOARD_BOOT_PIN, INPUT_PULLUP);

  DisplayEngine::begin();
  ConfigPortal::loadSettings();

  bool forcePortal = !ConfigPortal::hasValidSettings();
  if (!forcePortal) {
    auto gfx = DisplayEngine::getGfx();
    gfx->fillScreen(COL_BG_DARK);

    gfx->setTextColor(COL_CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(16, 20);
    gfx->print("CYD AEROSPACE RADAR");

    gfx->setTextSize(1);
    gfx->setTextColor(COL_WHITE);
    gfx->setCursor(16, 52);
    gfx->printf("Tracking: %s", ConfigPortal::getCityName());
    gfx->setCursor(16, 68);
    gfx->printf("Airspace: %.0f km (%.4f, %.4f)",
               ConfigPortal::getRadius(), ConfigPortal::getLat(), ConfigPortal::getLon());

    // Big interactive touch button for setup
    gfx->fillRoundRect(16, 92, 288, 56, 8, 0x0A54);
    gfx->drawRoundRect(16, 92, 288, 56, 8, COL_CYAN);
    gfx->setTextColor(COL_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(32, 106);
    gfx->print("TAP ANYWHERE TO OPEN WIFI SETUP");
    gfx->setTextColor(COL_YELLOW);
    gfx->setCursor(75, 124);
    gfx->print("(or press BOOT button)");

    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(16, 175);
    gfx->print("Starting live telemetry in ");

    unsigned long splashStart = millis();
    int lastSec = 4;
    while (millis() - splashStart < 3500) {
      int remainingSec = (3500 - (millis() - splashStart)) / 1000 + 1;
      if (remainingSec != lastSec && remainingSec >= 1) {
        lastSec = remainingSec;
        gfx->fillRect(180, 175, 40, 10, COL_BG_DARK);
        gfx->setTextColor(COL_CYAN);
        gfx->setCursor(180, 175);
        gfx->printf("%ds...", remainingSec);
      }

      int tx, ty;
      if (DisplayEngine::readTouch(tx, ty)) {
        forcePortal = true;
        break;
      }
      if (digitalRead(BOARD_BOOT_PIN) == LOW) {
        forcePortal = true;
        break;
      }
      delay(20);
    }
  }

  if (forcePortal) {
    ConfigPortal::runPortal();
  }

  DisplayEngine::getGfx()->fillScreen(COL_BG_DARK);
  DisplayEngine::showStatus("Connecting to Wi-Fi...", COL_CYAN);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ConfigPortal::getSsid(), ConfigPortal::getPass());

  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    DisplayEngine::showStatus("Synchronizing NTP atomic time...", COL_GREEN);
    configTzTime(ConfigPortal::getTimeZone(), "pool.ntp.org", "time.nist.gov");
    delay(800);
  } else {
    Serial.println("\n[WiFi] Connection failed! Starting in offline demo mode.");
    DisplayEngine::showStatus("Wi-Fi connection failed!", COL_RED);
    delay(1500);
  }

  fetchCityData();
}

void loop() {
  unsigned long now = millis();

  // Physical BOOT button (GPIO 28)
  if (digitalRead(BOARD_BOOT_PIN) == LOW) {
    delay(50);
    if (digitalRead(BOARD_BOOT_PIN) == LOW) {
      unsigned long pressStart = millis();
      while (digitalRead(BOARD_BOOT_PIN) == LOW && millis() - pressStart < 1500) {
        delay(20);
      }
      if (millis() - pressStart >= 1200) {
        DisplayEngine::showStatus("Opening WiFi / City Setup...", COL_YELLOW);
        delay(400);
        ConfigPortal::runPortal();
        DisplayEngine::getGfx()->fillScreen(COL_BG_DARK);
        fetchCityData();
      } else {
        currentMode = (AppMode)(((int)currentMode + 1) % MODE_COUNT);
        FlightListView::clearDetail();
        redrawCurrentView();
      }
      while (digitalRead(BOARD_BOOT_PIN) == LOW) delay(10);
    }
  }

  int tx, ty;
  if (DisplayEngine::readTouch(tx, ty)) {
    // 1. Top Header Bar Touches
    if (ty < HEADER_H + 4) {
      // Tapped [SET] on the right
      if (tx >= 260) {
        DisplayEngine::showStatus("Opening WiFi / City Setup...", COL_YELLOW);
        delay(400);
        ConfigPortal::runPortal();
        DisplayEngine::getGfx()->fillScreen(COL_BG_DARK);
        fetchCityData();
      }
      // Tapped City Name on the left
      else if (tx < 115) {
        if (ConfigPortal::isCustomCity()) {
          ConfigPortal::setCityPreset(0);
        } else {
          ConfigPortal::setCustomCity();
        }
        fetchCityData();
      }
    }
    // 2. Footer Navigation Bar
    else if (ty >= SCREEN_H - FOOTER_H) {
      int tabW = SCREEN_W / 6;
      int tabIdx = tx / tabW;

      if (tabIdx >= 0 && tabIdx < 5) {
        if (currentMode != (AppMode)tabIdx) {
          currentMode = (AppMode)tabIdx;
          FlightListView::clearDetail();
          redrawCurrentView();
        }
      } else if (tabIdx == 5) {
        ConfigPortal::nextCityPreset();
        fetchCityData();
      }
    }
    // 3. Content Area Touches
    else if (ty >= CONTENT_Y && ty < SCREEN_H - FOOTER_H) {
      if (currentMode == MODE_FLIGHT_LIST) {
        FlightListView::handleTouch(tx, ty);
      }
    }
  }

  if (WiFi.isConnected()) {
    if (now - lastOpenSkyFetch >= OPENSKY_REFRESH_MS) {
      lastOpenSkyFetch = now;
      OpenSkyClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon(), ConfigPortal::getRadius());
      if (currentMode == MODE_RADAR || currentMode == MODE_FLIGHT_LIST) {
        redrawCurrentView();
      }
    }

    if (now - lastWeatherFetch >= WEATHER_REFRESH_MS) {
      lastWeatherFetch = now;
      WeatherClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon());
      if (currentMode == MODE_WEATHER) {
        redrawCurrentView();
      }
    }

    if (now - lastIssFetch >= ISS_REFRESH_MS) {
      lastIssFetch = now;
      IssClient::fetch(ConfigPortal::getLat(), ConfigPortal::getLon());
      if (currentMode == MODE_ISS) {
        IssView::draw(ConfigPortal::getCityName());
      }
    }

    if (now - lastSpaceXFetch >= SPACEX_REFRESH_MS) {
      lastSpaceXFetch = now;
      SpaceXClient::fetch();
      if (currentMode == MODE_SPACEX) {
        redrawCurrentView();
      }
    }
  }

  if (currentMode == MODE_RADAR) {
    if (now - lastSweepTick >= SWEEP_ANIM_MS) {
      lastSweepTick = now;
      FlightRadarView::updateSweep(ConfigPortal::getRadius());
    }
  } else if (currentMode == MODE_SPACEX) {
    if (now - lastCountdownTick >= 1000) {
      lastCountdownTick = now;
      SpaceXView::updateCountdown();
    }
  }

  if (now - lastClockTick >= CLOCK_REFRESH_MS) {
    lastClockTick = now;
    updateClockString();
    DisplayEngine::drawHeader(ConfigPortal::getCityName(), timeString, OpenSkyClient::getCount(), WiFi.isConnected());
  }

  delay(10);
}
