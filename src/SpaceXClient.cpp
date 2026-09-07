#include "SpaceXClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <math.h>

SpaceXRecord SpaceXClient::spaceX = { false };

int64_t SpaceXClient::parseIsoToEpoch(const char *isoStr) {
  if (!isoStr || strlen(isoStr) < 19) return 0;
  struct tm tm_time;
  memset(&tm_time, 0, sizeof(struct tm));
  sscanf(isoStr, "%d-%d-%dT%d:%d:%d",
         &tm_time.tm_year, &tm_time.tm_mon, &tm_time.tm_mday,
         &tm_time.tm_hour, &tm_time.tm_min, &tm_time.tm_sec);
  tm_time.tm_year -= 1900;
  tm_time.tm_mon  -= 1;
  return (int64_t)mktime(&tm_time);
}

const SpaceXRecord &SpaceXClient::getData() {
  return spaceX;
}

bool SpaceXClient::fetch(float userLat, float userLon) {
  const char *url = "https://lldev.thespacedevs.com/2.2.0/launch/upcoming/?search=SpaceX&limit=1";

  Serial.println("[SpaceX] Fetching upcoming launch...");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(12000);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[SpaceX] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.printf("[SpaceX] JSON parse error: %s\n", err.c_str());
    return false;
  }

  JsonArray results = doc["results"].as<JsonArray>();
  if (results.size() == 0) {
    Serial.println("[SpaceX] No launches in results");
    return false;
  }

  JsonObject launch = results[0];

  const char *name = launch["name"] | "SpaceX Mission";
  strncpy(spaceX.mission_name, name, sizeof(spaceX.mission_name) - 1);
  spaceX.mission_name[sizeof(spaceX.mission_name) - 1] = '\0';

  const char *rName = launch["rocket"]["configuration"]["name"] | "Falcon 9";
  strncpy(spaceX.rocket_name, rName, sizeof(spaceX.rocket_name) - 1);
  spaceX.rocket_name[sizeof(spaceX.rocket_name) - 1] = '\0';

  JsonObject pad = launch["pad"];
  const char *pName = pad["name"] | "Space Launch Complex";
  strncpy(spaceX.pad_name, pName, sizeof(spaceX.pad_name) - 1);
  spaceX.pad_name[sizeof(spaceX.pad_name) - 1] = '\0';

  const char *lName = pad["location"]["name"] | "Cape Canaveral, FL";
  strncpy(spaceX.location_name, lName, sizeof(spaceX.location_name) - 1);
  spaceX.location_name[sizeof(spaceX.location_name) - 1] = '\0';

  spaceX.pad_lat = pad["latitude"].isNull() ? 0.0f : pad["latitude"].as<float>();
  spaceX.pad_lon = pad["longitude"].isNull() ? 0.0f : pad["longitude"].as<float>();

  // Calculate distance, bearing, and visibility from user location
  if (userLat != 0.0f && spaceX.pad_lat != 0.0f) {
    float dLat = (spaceX.pad_lat - userLat) * (float)M_PI / 180.0f;
    float dLon = (spaceX.pad_lon - userLon) * (float)M_PI / 180.0f;
    float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f) +
              cosf(userLat * (float)M_PI / 180.0f) * cosf(spaceX.pad_lat * (float)M_PI / 180.0f) *
              sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    spaceX.dist_km = 6371.0f * c;

    float y = sinf(dLon) * cosf(spaceX.pad_lat * (float)M_PI / 180.0f);
    float x = cosf(userLat * (float)M_PI / 180.0f) * sinf(spaceX.pad_lat * (float)M_PI / 180.0f) -
              sinf(userLat * (float)M_PI / 180.0f) * cosf(spaceX.pad_lat * (float)M_PI / 180.0f) * cosf(dLon);
    float b = atan2f(y, x) * 180.0f / (float)M_PI;
    if (b < 0.0f) b += 360.0f;
    spaceX.bearing = b;

    spaceX.visible_in_sky = (spaceX.dist_km <= 500.0f);
  } else {
    spaceX.dist_km = 0.0f;
    spaceX.bearing = 0.0f;
    spaceX.visible_in_sky = false;
  }

  const char *sName = launch["status"]["name"] | "Scheduled";
  strncpy(spaceX.status_name, sName, sizeof(spaceX.status_name) - 1);
  spaceX.status_name[sizeof(spaceX.status_name) - 1] = '\0';

  const char *net = launch["net"] | "";
  strncpy(spaceX.net_iso, net, sizeof(spaceX.net_iso) - 1);
  spaceX.net_iso[sizeof(spaceX.net_iso) - 1] = '\0';
  spaceX.launch_epoch_utc = parseIsoToEpoch(net);

  const char *desc = launch["mission"]["description"] | "Orbital launch operation.";
  strncpy(spaceX.details, desc, sizeof(spaceX.details) - 1);
  spaceX.details[sizeof(spaceX.details) - 1] = '\0';

  spaceX.valid = true;
  Serial.printf("[SpaceX] Launch: %s | Pad: %s, %s (%.0fkm, Visible: %d)\n",
                spaceX.mission_name, spaceX.pad_name, spaceX.location_name,
                spaceX.dist_km, spaceX.visible_in_sky);
  return true;
}
