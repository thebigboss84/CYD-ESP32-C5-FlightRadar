#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "AppConfig.h"
#include "DisplayEngine.h"
#include "ConfigPortal.h"
#include "GpsManager.h"
#include "OpenSkyClient.h"
#include "WeatherClient.h"
#include "IssClient.h"
#include "SpaceXClient.h"
#include "FlightRadarView.h"
#include "FlightListView.h"
#include "WeatherView.h"
#include "IssView.h"
#include "SpaceXView.h"
#include "LaunchAlertView.h"
#include "LedBeacon.h"
#include "SpeakerAlert.h"
#include "CitySelectView.h"
#include "SeismicClient.h"
#include "SeismicView.h"

static AppMode currentMode = MODE_RADAR;

static unsigned long lastOpenSkyFetch     = 0;
static unsigned long lastWeatherFetch     = 0;
static unsigned long lastIssFetch         = 0;
static unsigned long lastSpaceXFetch      = 0;
static unsigned long lastSeismicFetch     = 0;
static unsigned long lastClockTick        = 0;
static unsigned long lastSweepTick        = 0;
static unsigned long lastCountdownTick    = 0;
static unsigned long lastExtrapolateTick  = 0;
static unsigned long lastWifiRetry        = 0;
static bool wifiWasConnected              = false;

static const unsigned long WIFI_RETRY_MS = 30000UL;

static char timeString[24] = "12:00:00 AM";
static bool launchNoticeVisible = false;
static int64_t dismissedLaunchEpoch = 0;

static float getActiveLat() {
  if (CitySelectView::isUsingGps() && GpsManager::hasFix()) {
    return GpsManager::getLat();
  }
  return ConfigPortal::getLat();
}

static float getActiveLon() {
  if (CitySelectView::isUsingGps() && GpsManager::hasFix()) {
    return GpsManager::getLon();
  }
  return ConfigPortal::getLon();
}

static const char *getActiveCityName() {
  if (CitySelectView::isUsingGps() && GpsManager::hasFix()) {
    static char gpsCityBuf[24];
    snprintf(gpsCityBuf, sizeof(gpsCityBuf), "GPS: %s", ConfigPortal::getCityName());
    return gpsCityBuf;
  }
  return ConfigPortal::getCityName();
}

static void updateClockString() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeString, sizeof(timeString), "%I:%M:%S %p", &timeinfo);
  }
}

static void redrawCurrentView() {
  updateClockString();
  DisplayEngine::drawHeader(getActiveCityName(), timeString, OpenSkyClient::getCount(), WiFi.isConnected());

  switch (currentMode) {
    case MODE_RADAR:
      FlightRadarView::draw(ConfigPortal::getRadius());
      break;
    case MODE_FLIGHT_LIST:
      FlightListView::draw();
      break;
    case MODE_WEATHER:
      WeatherView::draw(getActiveCityName());
      break;
    case MODE_ISS:
      IssView::draw(getActiveCityName(), ConfigPortal::getRadius());
      break;
    case MODE_SPACEX:
      SpaceXView::draw();
      break;
    case MODE_CITY:
      CitySelectView::draw();
      break;
    case MODE_SEISMIC:
      SeismicView::draw();
      break;
    default:
      break;
  }

  DisplayEngine::drawFooter(currentMode);
}

static void fetchCityData() {
  DisplayEngine::showStatus("Acquiring airspace & telemetry...", COL_YELLOW);
  configTzTime(ConfigPortal::getTimeZone(), "pool.ntp.org", "time.nist.gov");

  float lat = getActiveLat();
  float lon = getActiveLon();

  OpenSkyClient::fetch(lat, lon, ConfigPortal::getRadius());
  WeatherClient::fetch(lat, lon);
  IssClient::fetch(lat, lon);
  SpaceXClient::fetch(lat, lon);
  SeismicClient::fetch(lat, lon);

  lastOpenSkyFetch = millis();
  lastWeatherFetch = millis();
  lastIssFetch     = millis();
  lastSpaceXFetch  = millis();
  lastSeismicFetch = millis();

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
  LedBeacon::begin();
  SpeakerAlert::begin();
  GpsManager::begin();
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
    gfx->fillRoundRect(16, 92, 288, 36, 6, 0x0861);
    gfx->drawRoundRect(16, 92, 288, 36, 6, COL_CYAN);
    gfx->setTextColor(COL_YELLOW);
    gfx->setTextSize(1);
    gfx->setCursor(32, 104);
    gfx->print("TAP HERE TO CONFIGURE / OPEN PORTAL");

    gfx->setTextColor(COL_GRAY);
    gfx->setCursor(16, 140);
    gfx->print("Booting live dashboard in 3 seconds...");

    unsigned long bootPromptStart = millis();
    while (millis() - bootPromptStart < 3000) {
      GpsManager::update();
      int tx, ty;
      if (DisplayEngine::readTouch(tx, ty)) {
        if (ty >= 80 && ty <= 140) {
          forcePortal = true;
          break;
        }
      }
      delay(50);
    }
  }

  if (forcePortal) {
    DisplayEngine::showStatus("Starting Setup Portal...", COL_CYAN);
    ConfigPortal::runPortal();
    Serial.println("[Setup] Portal finished. Proceeding with active configuration.");
  }

  // Connect to configured Wi-Fi
  DisplayEngine::showStatus("Connecting to Wi-Fi...", COL_CYAN);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ConfigPortal::getSsid(), ConfigPortal::getPass());

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 25) {
    GpsManager::update();
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    DisplayEngine::showStatus("Wi-Fi Connected! Syncing time...", COL_GREEN);
    configTzTime(ConfigPortal::getTimeZone(), "pool.ntp.org", "time.nist.gov");
    delay(1000);
    wifiWasConnected = true;
  } else {
    Serial.println("[WiFi] Connection failed! Starting Web Setup Portal...");
    DisplayEngine::showStatus("Wi-Fi failed. Opening portal...", COL_RED);
    ConfigPortal::runPortal();
  }

  // Fetch initial telemetry
  fetchCityData();
}

void loop() {
  unsigned long now = millis();

  // 1. Process GPS UART incoming sentences
  GpsManager::update();

  // If GPS acquired a 3D fix and moved significantly, update telemetry
  if (CitySelectView::isUsingGps() && GpsManager::hasMovedSignificantly(3.0f)) {
    Serial.printf("[GPS] Position update: %.4f, %.4f. Refreshing telemetry...\n",
                  GpsManager::getLat(), GpsManager::getLon());
    fetchCityData();
  }

  // 2. Hardware BOOT Button check
  if (digitalRead(BOARD_BOOT_PIN) == LOW) {
    delay(50);
    if (digitalRead(BOARD_BOOT_PIN) == LOW) {
      Serial.println("[Button] BOOT button pressed - cycling display mode...");
      currentMode = (AppMode)((currentMode + 1) % MODE_COUNT);
      FlightListView::clearDetail();
      redrawCurrentView();
      while (digitalRead(BOARD_BOOT_PIN) == LOW) delay(20);
    }
  }

  // 3. Touch Handling
  int tx, ty;
  if (DisplayEngine::readTouch(tx, ty)) {
    if (launchNoticeVisible) {
      if (LaunchAlertView::isCloseTouched(tx, ty)) {
        dismissedLaunchEpoch = SpaceXClient::getData().launch_epoch_utc;
        launchNoticeVisible = false;
        redrawCurrentView();
      }
    }
    // Top Bar Touches (Y: 0..HEADER_H)
    else if (ty < HEADER_H) {
      // Setup button [SET] touched (x: 260..320)
      if (tx >= 260) {
        Serial.println("[Touch] SETUP button pressed. Launching Config Portal...");
        ConfigPortal::runPortal();
        fetchCityData();
      }
    }
    // Footer Navigation Bar Touches (Y: SCREEN_H - FOOTER_H..SCREEN_H)
    else if (ty >= SCREEN_H - FOOTER_H) {
      int tabW = SCREEN_W / MODE_COUNT;
      int tabIdx = tx / tabW;

      if (tabIdx >= 0 && tabIdx < MODE_COUNT) {
        if (currentMode != (AppMode)tabIdx) {
          currentMode = (AppMode)tabIdx;
          FlightListView::clearDetail();
          redrawCurrentView();
        }
      }
    }
    // Content Area Touches
    else if (ty >= CONTENT_Y && ty < SCREEN_H - FOOTER_H) {
      if (currentMode == MODE_FLIGHT_LIST) {
        FlightListView::handleTouch(tx, ty);
      } else if (currentMode == MODE_RADAR) {
        if (FlightRadarView::checkSeismicBannerTouch(tx, ty)) {
          currentMode = MODE_SEISMIC;
          redrawCurrentView();
        } else {
          FlightRadarView::handleTouch(tx, ty);
        }
      } else if (currentMode == MODE_CITY) {
        if (CitySelectView::handleTouch(tx, ty)) {
          fetchCityData();
        }
      } else if (currentMode == MODE_SEISMIC) {
        if (SeismicView::handleTouch(tx, ty)) {
          fetchCityData();
        }
      }
    }
  }

  // 4. Periodic API Data Fetching
  bool wifiConnected = WiFi.status() == WL_CONNECTED;
  if (!wifiConnected) {
    if (wifiWasConnected) {
      wifiWasConnected = false;
      Serial.println("[WiFi] Connection lost. Retrying in the background.");
    }
    if (now - lastWifiRetry >= WIFI_RETRY_MS) {
      lastWifiRetry = now;
      Serial.println("[WiFi] Attempting reconnect...");
      WiFi.disconnect();
      WiFi.begin(ConfigPortal::getSsid(), ConfigPortal::getPass());
    }
  } else if (!wifiWasConnected) {
    wifiWasConnected = true;
    Serial.println("[WiFi] Reconnected. Refreshing telemetry...");
    configTzTime(ConfigPortal::getTimeZone(), "pool.ntp.org", "time.nist.gov");
    fetchCityData();
  }

  if (wifiConnected) {
    if (now - lastOpenSkyFetch >= OPENSKY_REFRESH_MS) {
      lastOpenSkyFetch = now;
      OpenSkyClient::fetch(getActiveLat(), getActiveLon(), ConfigPortal::getRadius());
      if (currentMode == MODE_RADAR || currentMode == MODE_FLIGHT_LIST) {
        redrawCurrentView();
      }
    }

    if (now - lastWeatherFetch >= WEATHER_REFRESH_MS) {
      lastWeatherFetch = now;
      WeatherClient::fetch(getActiveLat(), getActiveLon());
      if (currentMode == MODE_WEATHER) {
        redrawCurrentView();
      }
    }

    if (now - lastIssFetch >= ISS_REFRESH_MS) {
      lastIssFetch = now;
      IssClient::fetch(getActiveLat(), getActiveLon());
    if (currentMode == MODE_ISS) {
        IssView::draw(getActiveCityName(), ConfigPortal::getRadius());
      }
    }

    if (now - lastSpaceXFetch >= SPACEX_REFRESH_MS) {
      lastSpaceXFetch = now;
      SpaceXClient::fetch(getActiveLat(), getActiveLon());
      if (currentMode == MODE_SPACEX) {
        redrawCurrentView();
      }
    }

    if (now - lastSeismicFetch >= SEISMIC_REFRESH_MS) {
      lastSeismicFetch = now;
      SeismicClient::fetch(getActiveLat(), getActiveLon());
      if (currentMode == MODE_SEISMIC || (currentMode == MODE_RADAR && SeismicClient::hasActiveAlert())) {
        redrawCurrentView();
      }
    }
  }

  // 5. Radar Display Animations & Dead Reckoning
  if (currentMode == MODE_RADAR) {
    // Paced tactical sweep beam animation (every SWEEP_ANIM_MS, e.g. 80ms)
    if (now - lastSweepTick >= SWEEP_ANIM_MS) {
      lastSweepTick = now;
      FlightRadarView::updateSweep(ConfigPortal::getRadius());
    }

    // Aircraft dead-reckoning extrapolation (every EXTRAPOLATE_MS, e.g. 2000ms)
    if (now - lastExtrapolateTick >= EXTRAPOLATE_MS) {
      lastExtrapolateTick = now;
      OpenSkyClient::extrapolatePositions(EXTRAPOLATE_MS / 1000.0f, getActiveLat(), getActiveLon());
      FlightRadarView::draw(ConfigPortal::getRadius());
    }
  } else if (currentMode == MODE_SPACEX) {
    if (now - lastCountdownTick >= 1000) {
      lastCountdownTick = now;
      SpaceXView::updateCountdown();
    }
  }

  // 6. Clock Update
  if (now - lastClockTick >= CLOCK_REFRESH_MS) {
    lastClockTick = now;
    updateClockString();
    DisplayEngine::drawHeader(getActiveCityName(), timeString, OpenSkyClient::getCount(), WiFi.isConnected());
  }

  // 7. WS2812 Aerospace RGB Beacon Updates
  LedBeacon::setEmergency(OpenSkyClient::hasActiveEmergency());
  LedBeacon::setAircraftOverhead(OpenSkyClient::hasAircraftOverhead(6.0f));
  const SeismicRecord &quake = SeismicClient::getLatest();
  LedBeacon::setSeismicAlert(SeismicClient::hasActiveAlert(), quake.mag);

  const SpaceXRecord &sp = SpaceXClient::getData();
  int64_t diff = 0;
  if (sp.valid && sp.launch_epoch_utc > 0) {
    time_t nowUtc = time(nullptr);
    diff = sp.launch_epoch_utc - (int64_t)nowUtc;
    LedBeacon::setSpaceXState(sp.visible_in_sky && diff > 0 && diff <= 900,
                              sp.visible_in_sky && diff <= 0 && diff >= -600);
  } else {
    LedBeacon::setSpaceXState(false, false);
  }

  bool launchNoticeDue = sp.valid && sp.launch_epoch_utc > 0 &&
                         diff > 0 && diff <= SPACEX_NOTICE_WINDOW_SEC;
  bool launchNoticeWasVisible = launchNoticeVisible;
  if (launchNoticeDue && sp.launch_epoch_utc != dismissedLaunchEpoch) {
    launchNoticeVisible = true;
  }
  if (launchNoticeVisible && (!sp.valid || diff <= 0 ||
      sp.launch_epoch_utc == dismissedLaunchEpoch)) {
    launchNoticeVisible = false;
    redrawCurrentView();
  }

  const IssRecord &iss = IssClient::getData();
  LedBeacon::setIssPass(iss.valid && iss.dist_km <= 800.0f && iss.daylight);

  SpeakerAlert::update(SeismicClient::hasActiveAlert(), quake.id,
                       sp.valid && diff > 0 && diff <= SPACEX_NOTICE_WINDOW_SEC,
                       sp.valid && diff <= 0 && diff >= -600,
                       sp.launch_epoch_utc);

  if (launchNoticeVisible && (!launchNoticeWasVisible ||
      now - lastCountdownTick >= CLOCK_REFRESH_MS)) {
    lastCountdownTick = now;
    LaunchAlertView::draw(sp, diff);
  }

  LedBeacon::update();

  delay(10);
}
