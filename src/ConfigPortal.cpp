#include "ConfigPortal.h"
#include "DisplayEngine.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

char  ConfigPortal::ssid[64]           = "";
char  ConfigPortal::pass[64]           = "";
char  ConfigPortal::cityName[32]       = "Rancho Cucamonga";
float ConfigPortal::lat                = 34.1233f;
float ConfigPortal::lon                = -117.5794f;
float ConfigPortal::radiusKm           = 150.0f;
int   ConfigPortal::currentCityIdx     = -1;
bool  ConfigPortal::configured         = false;

char  ConfigPortal::customCityName[32] = "Rancho Cucamonga";
float ConfigPortal::customLat          = 34.1233f;
float ConfigPortal::customLon          = -117.5794f;
bool  ConfigPortal::hasCustomCity      = true;
bool  ConfigPortal::usingCustom        = true;

static Preferences prefs;
static DNSServer *dnsServer = nullptr;
static WebServer *webServer = nullptr;
static bool portalActive = false;

void ConfigPortal::loadSettings() {
  prefs.begin("aerocyd", false);
  String s = prefs.getString("ssid", "");
  String p = prefs.getString("pass", "");
  String c = prefs.getString("city", "");
  lat      = prefs.getFloat ("lat", 34.1233f);
  lon      = prefs.getFloat ("lon", -117.5794f);
  radiusKm = prefs.getFloat ("radius", 150.0f);
  currentCityIdx = prefs.getInt("city_idx", -1);

  hasCustomCity = prefs.getBool("has_custom", false);
  usingCustom   = prefs.getBool("using_custom", true);
  String cc     = prefs.getString("custom_city", "");
  customLat     = prefs.getFloat ("custom_lat", 0.0f);
  customLon     = prefs.getFloat ("custom_lon", 0.0f);
  prefs.end();

  s.trim();
  p.trim();
  c.trim();
  cc.trim();

  strncpy(ssid, s.c_str(), sizeof(ssid) - 1);
  strncpy(pass, p.c_str(), sizeof(pass) - 1);
  strncpy(cityName, c.c_str(), sizeof(cityName) - 1);
  strncpy(customCityName, cc.c_str(), sizeof(customCityName) - 1);

  // If custom city is not explicitly stored yet in custom_city:
  if (!hasCustomCity || strlen(customCityName) == 0) {
    bool matchesPreset = false;
    for (size_t i = 0; i < CITY_PRESETS_COUNT; i++) {
      if (strcasecmp(cityName, CITY_PRESETS[i].name) == 0) {
        matchesPreset = true;
        break;
      }
    }

    if (!matchesPreset && strlen(cityName) > 0) {
      strncpy(customCityName, cityName, sizeof(customCityName) - 1);
      customLat = lat;
      customLon = lon;
    } else {
      strncpy(customCityName, "Rancho Cucamonga", sizeof(customCityName) - 1);
      customLat = 34.1233f;
      customLon = -117.5794f;
    }
    hasCustomCity = true;
    usingCustom = true;
    currentCityIdx = -1;

    // Persist to NVS so it is permanently preserved
    prefs.begin("aerocyd", false);
    prefs.putBool("has_custom", true);
    prefs.putBool("using_custom", true);
    prefs.putString("custom_city", customCityName);
    prefs.putFloat("custom_lat", customLat);
    prefs.putFloat("custom_lon", customLon);
    prefs.putString("city", customCityName);
    prefs.putFloat("lat", customLat);
    prefs.putFloat("lon", customLon);
    prefs.putInt("city_idx", -1);
    prefs.end();
  }

  if (usingCustom) {
    strncpy(cityName, customCityName, sizeof(cityName) - 1);
    lat = customLat;
    lon = customLon;
  }

  configured = (strlen(ssid) > 0);
  Serial.printf("[Config] Loaded: WiFi='%s', City='%s' (%.4f, %.4f), Custom=%d, Home='%s', Radius=%.0fkm\n",
                ssid, cityName, lat, lon, usingCustom, customCityName, radiusKm);
}

void ConfigPortal::saveSettings(const char *s, const char *p, const char *c, float la, float lo, float rad, bool isCustom) {
  String strCity = String(c);
  strCity.trim();
  String strSsid = String(s);
  strSsid.trim();
  String strPass = String(p);
  strPass.trim();

  strncpy(ssid, strSsid.c_str(), sizeof(ssid) - 1);
  strncpy(pass, strPass.c_str(), sizeof(pass) - 1);
  strncpy(cityName, strCity.c_str(), sizeof(cityName) - 1);
  lat = la;
  lon = lo;
  radiusKm = rad;
  configured = true;

  if (isCustom) {
    hasCustomCity = true;
    usingCustom   = true;
    currentCityIdx = -1;
    strncpy(customCityName, strCity.c_str(), sizeof(customCityName) - 1);
    customLat = la;
    customLon = lo;
  } else {
    usingCustom = false;
  }

  prefs.begin("aerocyd", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.putString("city", cityName);
  prefs.putFloat ("lat", lat);
  prefs.putFloat ("lon", lon);
  prefs.putFloat ("radius", radiusKm);
  prefs.putInt   ("city_idx", currentCityIdx);
  prefs.putBool  ("has_custom", hasCustomCity);
  prefs.putBool  ("using_custom", usingCustom);
  prefs.putString("custom_city", customCityName);
  prefs.putFloat ("custom_lat", customLat);
  prefs.putFloat ("custom_lon", customLon);
  prefs.end();

  Serial.printf("[Config] Saved: City='%s' (%.4f, %.4f), Custom=%d, Home='%s'\n",
                cityName, lat, lon, usingCustom, customCityName);
}

void ConfigPortal::setCityPreset(int index) {
  if (index >= 0 && index < (int)CITY_PRESETS_COUNT) {
    usingCustom = false;
    currentCityIdx = index;
    const CityPreset &p = CITY_PRESETS[index];
    strncpy(cityName, p.name, sizeof(cityName) - 1);
    lat = p.lat;
    lon = p.lon;

    prefs.begin("aerocyd", false);
    prefs.putString("city", cityName);
    prefs.putFloat ("lat", lat);
    prefs.putFloat ("lon", lon);
    prefs.putInt   ("city_idx", currentCityIdx);
    prefs.putBool  ("using_custom", false);
    // Custom city keys remain completely intact in NVS
    prefs.end();

    Serial.printf("[Config] Switched to preset: %s (%.4f, %.4f)\n", cityName, lat, lon);
  }
}

void ConfigPortal::setCustomCity() {
  if (hasCustomCity && strlen(customCityName) > 0) {
    usingCustom = true;
    currentCityIdx = -1;
    strncpy(cityName, customCityName, sizeof(cityName) - 1);
    lat = customLat;
    lon = customLon;

    prefs.begin("aerocyd", false);
    prefs.putString("city", cityName);
    prefs.putFloat ("lat", lat);
    prefs.putFloat ("lon", lon);
    prefs.putInt   ("city_idx", -1);
    prefs.putBool  ("using_custom", true);
    prefs.end();

    Serial.printf("[Config] Switched to Custom Home City: %s (%.4f, %.4f)\n", cityName, lat, lon);
  } else {
    setCityPreset(0);
  }
}

void ConfigPortal::nextCityPreset() {
  if (usingCustom) {
    setCityPreset(0);
  } else if (currentCityIdx + 1 < (int)CITY_PRESETS_COUNT) {
    setCityPreset(currentCityIdx + 1);
  } else {
    setCustomCity();
  }
}

const char *ConfigPortal::getSsid()           { return ssid; }
const char *ConfigPortal::getPass()           { return pass; }
const char *ConfigPortal::getCityName()       { return cityName; }
const char *ConfigPortal::getCustomCityName() { return customCityName; }
float ConfigPortal::getLat()                  { return lat; }
float ConfigPortal::getLon()                  { return lon; }
float ConfigPortal::getRadius()               { return radiusKm; }
bool  ConfigPortal::hasValidSettings()        { return configured; }
bool  ConfigPortal::isCustomCity()            { return usingCustom; }
int   ConfigPortal::getCityPresetIndex()      { return currentCityIdx; }

const char *ConfigPortal::getTimeZone() {
  if (usingCustom || currentCityIdx < 0 || currentCityIdx >= (int)CITY_PRESETS_COUNT) {
    // Pacific Time for Rancho Cucamonga (PST8PDT with daylight saving time)
    return "PST8PDT,M3.2.0,M11.1.0";
  }
  return CITY_PRESETS[currentCityIdx].tz;
}

void ConfigPortal::stopPortal() {
  portalActive = false;
}

static void handlePortalRoot() {
  String defCity = String(ConfigPortal::hasValidSettings() ? ConfigPortal::getCityName() : "Rancho Cucamonga");
  float defLat   = ConfigPortal::getLat();
  float defLon   = ConfigPortal::getLon();
  float defRad   = ConfigPortal::getRadius();

  String html =
    "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>CYD Aerospace Setup</title>"
    "<style>"
    "body{background:#000d1a;color:#00e5ff;font-family:-apple-system,sans-serif;text-align:center;padding:16px;max-width:440px;margin:auto;}"
    "h2{color:#00ffff;margin-bottom:6px;font-weight:700;}"
    "p{color:#88aacc;font-size:0.9em;}"
    "label{display:block;text-align:left;margin:12px 0 4px;color:#88ddff;font-size:0.85em;font-weight:bold;}"
    "input,select{width:100%;box-sizing:border-box;background:#001529;color:#00e5ff;border:1.5px solid #005588;border-radius:6px;padding:10px;font-size:1em;}"
    ".btn{display:block;width:100%;padding:13px;margin:20px 0;font-size:1.05em;border-radius:8px;border:none;cursor:pointer;font-weight:bold;background:#0077cc;color:#ffffff;}"
    ".btn:hover{background:#0099ff;}"
    ".card{background:#001f3f;border:1px solid #005588;border-radius:8px;padding:12px;margin:12px 0;text-align:left;}"
    "</style></head><body>"
    "<h2>&#9992; CYD Aerospace Radar</h2>"
    "<p>Configure Wi-Fi connection, home city, and radar scan range.</p>"
    "<form method='POST' action='/save'>"
    "<label>Wi-Fi SSID:</label>"
    "<input type='text' name='ssid' value='" + String(ConfigPortal::getSsid()) + "' required>"
    "<label>Wi-Fi Password:</label>"
    "<input type='password' name='pass' value='" + String(ConfigPortal::getPass()) + "'>"
    "<div class='card'>"
    "<label>Select City Preset (Optional):</label>"
    "<select name='city_preset' onchange=\"if(this.value!='custom'){var p=this.value.split(',');document.getElementById('cname').value=p[0];document.getElementById('clat').value=p[1];document.getElementById('clon').value=p[2];}\">"
    "<option value='custom'>-- Custom Home City --</option>";

  for (size_t i = 0; i < CITY_PRESETS_COUNT; i++) {
    const CityPreset &cp = CITY_PRESETS[i];
    String optVal = String(cp.name) + "," + String(cp.lat, 4) + "," + String(cp.lon, 4);
    html += "<option value='" + optVal + "'>" + String(cp.name) + "</option>";
  }

  html +=
    "</select>"
    "<label>City / Location Name:</label>"
    "<input type='text' id='cname' name='city' value='" + defCity + "' required>"
    "<label>Latitude:</label>"
    "<input type='text' id='clat' name='lat' value='" + String(defLat, 4) + "' required>"
    "<label>Longitude:</label>"
    "<input type='text' id='clon' name='lon' value='" + String(defLon, 4) + "' required>"
    "</div>"
    "<label>Radar Scan Radius (km):</label>"
    "<select name='radius'>"
    "<option value='50'"  + String((defRad == 50)  ? " selected" : "") + ">50 km (Local Airspace)</option>"
    "<option value='100'" + String((defRad == 100) ? " selected" : "") + ">100 km (Metro / Regional)</option>"
    "<option value='150'" + String((defRad == 150) ? " selected" : "") + ">150 km (Standard Range)</option>"
    "<option value='250'" + String((defRad == 250) ? " selected" : "") + ">250 km (Long Range)</option>"
    "</select>"
    "<button class='btn' type='submit'>Save & Launch Instrument</button>"
    "</form></body></html>";

  webServer->send(200, "text/html", html);
}

static void handlePortalSave() {
  String s   = webServer->arg("ssid");
  String p   = webServer->arg("pass");
  String c   = webServer->arg("city");
  float  lat = webServer->arg("lat").toFloat();
  float  lon = webServer->arg("lon").toFloat();
  float  rad = webServer->arg("radius").toFloat();

  c.trim();
  bool isCustom = true;
  for (size_t i = 0; i < CITY_PRESETS_COUNT; i++) {
    if (strcasecmp(c.c_str(), CITY_PRESETS[i].name) == 0 &&
        fabs(lat - CITY_PRESETS[i].lat) < 0.01f) {
      isCustom = false;
      break;
    }
  }

  ConfigPortal::saveSettings(s.c_str(), p.c_str(), c.c_str(), lat, lon, rad, isCustom);

  webServer->send(200, "text/html",
    "<html><body style='background:#000d1a;color:#00ffff;font-family:sans-serif;text-align:center;padding:40px;'>"
    "<h2>&#9989; Settings Saved!</h2>"
    "<p>Connecting to Wi-Fi network and tracking: <b>" + c + "</b></p>"
    "<p style='color:#88aacc;'>Instrument is restarting now...</p>"
    "</body></html>");

  delay(1200);
  ESP.restart();
}

void ConfigPortal::runPortal() {
  auto gfx = DisplayEngine::getGfx();
  gfx->fillScreen(COL_BG_DARK);

  gfx->setTextColor(COL_CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(16, 16);
  gfx->print("CYD Aerospace Setup");

  gfx->setTextSize(1);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(16, 44);
  gfx->print("1. Connect phone/PC to Wi-Fi AP:");

  gfx->setTextColor(COL_YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(24, 58);
  gfx->print("CYD_Aerospace_Setup");

  gfx->setTextSize(1);
  gfx->setTextColor(COL_WHITE);
  gfx->setCursor(16, 88);
  gfx->print("2. Open browser and go to:");

  gfx->setTextColor(COL_CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(24, 102);
  gfx->print("http://192.168.4.1");

  gfx->setTextSize(1);
  gfx->setTextColor(COL_GRAY);
  gfx->setCursor(16, 134);
  gfx->print("Configure home Wi-Fi & custom city.");

  // Cancel / Exit button on touchscreen
  gfx->fillRoundRect(24, 162, 272, 44, 8, 0x0842);
  gfx->drawRoundRect(24, 162, 272, 44, 8, COL_YELLOW);
  gfx->setTextColor(COL_YELLOW);
  gfx->setTextSize(1);
  gfx->setCursor(52, 178);
  gfx->print("TAP HERE TO EXIT / RESUME RADAR");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("CYD_Aerospace_Setup", "");
  delay(300);

  dnsServer = new DNSServer();
  webServer = new WebServer(80);

  dnsServer->start(53, "*", WiFi.softAPIP());
  webServer->on("/", HTTP_GET, handlePortalRoot);
  webServer->on("/save", HTTP_POST, handlePortalSave);
  webServer->onNotFound(handlePortalRoot);
  webServer->begin();

  portalActive = true;
  while (portalActive) {
    dnsServer->processNextRequest();
    webServer->handleClient();

    int tx, ty;
    if (DisplayEngine::readTouch(tx, ty)) {
      if (ty >= 150) {
        portalActive = false;
        break;
      }
    }
    if (digitalRead(BOARD_BOOT_PIN) == LOW) {
      delay(50);
      if (digitalRead(BOARD_BOOT_PIN) == LOW) {
        portalActive = false;
        while (digitalRead(BOARD_BOOT_PIN) == LOW) delay(10);
        break;
      }
    }
    delay(5);
  }

  webServer->stop();
  dnsServer->stop();
  WiFi.softAPdisconnect(true);
  delete webServer; webServer = nullptr;
  delete dnsServer; dnsServer = nullptr;
}
