#include "SeismicClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include <time.h>

SeismicRecord SeismicClient::records[MAX_SEISMIC_EVENTS];
int SeismicClient::recordCount = 0;
bool SeismicClient::alertActive = false;

static float toRadians(float deg) {
  return deg * (float)M_PI / 180.0f;
}

float SeismicClient::calculateDistanceKm(float lat1, float lon1, float lat2, float lon2) {
  float dLat = toRadians(lat2 - lat1);
  float dLon = toRadians(lon2 - lon1);
  float rLat1 = toRadians(lat1);
  float rLat2 = toRadians(lat2);

  float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f) +
            cosf(rLat1) * cosf(rLat2) * sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
  float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
  return 6371.0f * c;
}

float SeismicClient::calculateBearingDeg(float lat1, float lon1, float lat2, float lon2) {
  float dLon = toRadians(lon2 - lon1);
  float rLat1 = toRadians(lat1);
  float rLat2 = toRadians(lat2);

  float y = sinf(dLon) * cosf(rLat2);
  float x = cosf(rLat1) * sinf(rLat2) - sinf(rLat1) * cosf(rLat2) * cosf(dLon);
  float bRad = atan2f(y, x);
  float bDeg = bRad * 180.0f / (float)M_PI;
  if (bDeg < 0.0f) bDeg += 360.0f;
  return bDeg;
}

bool SeismicClient::hasActiveAlert() {
  return alertActive;
}

const SeismicRecord &SeismicClient::getLatest() {
  static SeismicRecord empty = {};
  if (recordCount > 0) {
    return records[0];
  }
  return empty;
}

int SeismicClient::getCount() {
  return recordCount;
}

const SeismicRecord *SeismicClient::getRecord(int index) {
  if (index >= 0 && index < recordCount) {
    return &records[index];
  }
  return nullptr;
}

bool SeismicClient::fetch(float userLat, float userLon) {
  char url[256];
  snprintf(url, sizeof(url),
           "https://earthquake.usgs.gov/fdsnws/event/1/query?format=geojson&latitude=%.4f&longitude=%.4f&maxradiuskm=200&minmagnitude=1.5&orderby=time&limit=5",
           userLat, userLon);

  Serial.printf("[USGS] Querying seismic events within 200km of (%.4f, %.4f)...\n", userLat, userLon);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(12000);
  http.addHeader("User-Agent", "CYD-ESP32-AeroRadar/1.0");

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[USGS] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("[USGS] JSON parse error: %s\n", err.c_str());
    return false;
  }

  JsonArray features = doc["features"].as<JsonArray>();
  recordCount = 0;
  alertActive = false;

  time_t nowSec = time(nullptr);
  int64_t nowMs = (int64_t)nowSec * 1000LL;

  for (JsonObject f : features) {
    if (recordCount >= MAX_SEISMIC_EVENTS) break;

    JsonObject props = f["properties"];
    JsonObject geom  = f["geometry"];
    JsonArray coords = geom["coordinates"];

    SeismicRecord &rec = records[recordCount];
    memset(&rec, 0, sizeof(SeismicRecord));

    rec.valid = true;
    const char *idStr = f["id"] | "";
    strncpy(rec.id, idStr, sizeof(rec.id) - 1);

    rec.mag = props["mag"] | 0.0f;

    const char *placeStr = props["place"] | "Unknown location";
    strncpy(rec.place, placeStr, sizeof(rec.place) - 1);

    rec.epoch_ms = props["time"] | 0LL;

    // Coordinates: [longitude, latitude, depth_km]
    rec.lon = coords[0] | 0.0f;
    rec.lat = coords[1] | 0.0f;
    rec.depth_km = coords[2] | 0.0f;

    rec.dist_km = calculateDistanceKm(userLat, userLon, rec.lat, rec.lon);
    rec.bearing = calculateBearingDeg(userLat, userLon, rec.lat, rec.lon);

    if (nowMs > rec.epoch_ms) {
      rec.age_min = (int)((nowMs - rec.epoch_ms) / 60000LL);
    } else {
      rec.age_min = 0;
    }

    // Alert criteria: M >= 2.0, within 200 km, occurred within last 90 minutes
    if (rec.mag >= 2.0f && rec.dist_km <= 200.0f && rec.age_min <= 90) {
      rec.is_alert = true;
      alertActive = true;
    }

    Serial.printf("[USGS] Tremor: M%.1f | %s | Dist: %.1fkm, Hdg: %.0f deg | Depth: %.1fkm | Age: %dm | Alert: %d\n",
                  rec.mag, rec.place, rec.dist_km, rec.bearing, rec.depth_km, rec.age_min, rec.is_alert);

    recordCount++;
  }

  Serial.printf("[USGS] Updated %d events. Active Alert: %s\n", recordCount, alertActive ? "YES (!)" : "No");
  return true;
}
