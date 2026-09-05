#include "IssClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>

IssRecord IssClient::iss = { false };

float IssClient::calcHaversine(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0f;
  float dLat = (lat2 - lat1) * (float)M_PI / 180.0f;
  float dLon = (lon2 - lon1) * (float)M_PI / 180.0f;
  float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f)
          + cosf(lat1 * (float)M_PI / 180.0f) * cosf(lat2 * (float)M_PI / 180.0f)
          * sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
  return R * 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
}

float IssClient::calcBearing(float lat1, float lon1, float lat2, float lon2) {
  float l1 = lat1 * (float)M_PI / 180.0f;
  float l2 = lat2 * (float)M_PI / 180.0f;
  float dl = (lon2 - lon1) * (float)M_PI / 180.0f;
  float x  = sinf(dl) * cosf(l2);
  float y  = cosf(l1) * sinf(l2) - sinf(l1) * cosf(l2) * cosf(dl);
  return fmodf(atan2f(x, y) * 180.0f / (float)M_PI + 360.0f, 360.0f);
}

const IssRecord &IssClient::getData() {
  return iss;
}

bool IssClient::fetch(float userLat, float userLon) {
  const char *url = "https://api.wheretheiss.at/v1/satellites/25544";

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(8000);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[ISS] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.printf("[ISS] JSON parse error: %s\n", err.c_str());
    return false;
  }

  iss.lat          = doc["latitude"] | 0.0f;
  iss.lon          = doc["longitude"] | 0.0f;
  iss.alt_km       = doc["altitude"] | 420.0f;
  iss.velocity_kmh = doc["velocity"] | 27600.0f;
  const char *vis  = doc["visibility"] | "daylight";
  iss.daylight     = (strcmp(vis, "daylight") == 0);
  iss.timestamp    = doc["timestamp"] | 0;

  iss.dist_km = calcHaversine(userLat, userLon, iss.lat, iss.lon);
  iss.bearing = calcBearing(userLat, userLon, iss.lat, iss.lon);
  iss.valid   = true;

  Serial.printf("[ISS] Pos: %.2f, %.2f | Dist: %.0fkm | Hdg: %.0f\n",
                iss.lat, iss.lon, iss.dist_km, iss.bearing);
  return true;
}
