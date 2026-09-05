#include "OpenSkyClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>

FlightRecord OpenSkyClient::flights[MAX_TRACKED_FLIGHTS];
int OpenSkyClient::flightCount = 0;

float OpenSkyClient::metersToFeet(float m) {
  return m * 3.28084f;
}

float OpenSkyClient::msToKnots(float ms) {
  return ms * 1.94384f;
}

float OpenSkyClient::calcHaversine(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0f;
  float dLat = (lat2 - lat1) * (float)M_PI / 180.0f;
  float dLon = (lon2 - lon1) * (float)M_PI / 180.0f;
  float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f)
          + cosf(lat1 * (float)M_PI / 180.0f) * cosf(lat2 * (float)M_PI / 180.0f)
          * sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
  return R * 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
}

float OpenSkyClient::calcBearing(float lat1, float lon1, float lat2, float lon2) {
  float l1 = lat1 * (float)M_PI / 180.0f;
  float l2 = lat2 * (float)M_PI / 180.0f;
  float dl = (lon2 - lon1) * (float)M_PI / 180.0f;
  float x  = sinf(dl) * cosf(l2);
  float y  = cosf(l1) * sinf(l2) - sinf(l1) * cosf(l2) * cosf(dl);
  return fmodf(atan2f(x, y) * 180.0f / (float)M_PI + 360.0f, 360.0f);
}

const char *OpenSkyClient::getCompassDir(float b) {
  static const char *dirs[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW", "N" };
  int idx = (int)((b + 22.5f) / 45.0f) % 8;
  return dirs[idx];
}

void OpenSkyClient::insertSorted(const FlightRecord &rec) {
  if (flightCount < MAX_TRACKED_FLIGHTS) {
    int pos = flightCount;
    for (int i = 0; i < flightCount; i++) {
      if (rec.dist_km < flights[i].dist_km) {
        pos = i;
        break;
      }
    }
    for (int i = flightCount; i > pos; i--) {
      flights[i] = flights[i - 1];
    }
    flights[pos] = rec;
    flightCount++;
  } else if (rec.dist_km < flights[MAX_TRACKED_FLIGHTS - 1].dist_km) {
    int pos = MAX_TRACKED_FLIGHTS - 1;
    for (int i = 0; i < MAX_TRACKED_FLIGHTS; i++) {
      if (rec.dist_km < flights[i].dist_km) {
        pos = i;
        break;
      }
    }
    for (int i = MAX_TRACKED_FLIGHTS - 1; i > pos; i--) {
      flights[i] = flights[i - 1];
    }
    flights[pos] = rec;
  }
}

int OpenSkyClient::getCount() {
  return flightCount;
}

const FlightRecord *OpenSkyClient::getFlight(int index) {
  if (index >= 0 && index < flightCount) return &flights[index];
  return nullptr;
}

void OpenSkyClient::fetchRoute(int index) {
  if (index < 0 || index >= flightCount) return;
  FlightRecord &f = flights[index];
  if (f.route_fetched) return;
  f.route_fetched = true;

  if (strlen(f.callsign) < 3) return;

  char url[96];
  snprintf(url, sizeof(url), "http://api.adsbdb.com/v0/callsign/%s", f.callsign);

  HTTPClient http;
  http.begin(url);
  http.setTimeout(2000);

  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      JsonObject route = doc["response"]["flightroute"];
      if (!route.isNull()) {
        const char *orig = route["origin"]["iata_code"] | route["origin"]["icao_code"] | "";
        const char *dest = route["destination"]["iata_code"] | route["destination"]["icao_code"] | "";
        const char *airl = route["airline"]["name"] | "";
        const char *ac   = doc["response"]["aircraft"]["type"] | "";

        strncpy(f.origin, orig, sizeof(f.origin) - 1);
        f.origin[sizeof(f.origin) - 1] = '\0';

        strncpy(f.dest, dest, sizeof(f.dest) - 1);
        f.dest[sizeof(f.dest) - 1] = '\0';

        strncpy(f.airline, airl, sizeof(f.airline) - 1);
        f.airline[sizeof(f.airline) - 1] = '\0';

        strncpy(f.aircraft, ac, sizeof(f.aircraft) - 1);
        f.aircraft[sizeof(f.aircraft) - 1] = '\0';

        Serial.printf("[ADSBdb] Route for %s: %s -> %s (%s)\n", f.callsign, f.origin, f.dest, f.airline);
      }
    }
  }
  http.end();
}

bool OpenSkyClient::fetch(float userLat, float userLon, float radiusKm) {
  float degLat = radiusKm / 111.0f;
  float degLon = radiusKm / (111.0f * cosf(userLat * (float)M_PI / 180.0f));

  char url[256];
  snprintf(url, sizeof(url),
    "https://opensky-network.org/api/states/all?lamin=%.4f&lomin=%.4f&lamax=%.4f&lomax=%.4f",
    userLat - degLat, userLon - degLon,
    userLat + degLat, userLon + degLon);

  Serial.printf("[OpenSky] Fetching: %s\n", url);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(15000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[OpenSky] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.printf("[OpenSky] JSON parse failed: %s\n", err.c_str());
    return false;
  }

  JsonArray states = doc["states"].as<JsonArray>();
  flightCount = 0;

  if (states.isNull()) {
    Serial.println("[OpenSky] No aircraft returned");
    return true;
  }

  for (JsonArray state : states) {
    if (state[5].isNull() || state[6].isNull()) continue;

    float lon = state[5].as<float>();
    float lat = state[6].as<float>();
    float dist = calcHaversine(userLat, userLon, lat, lon);
    if (dist > radiusKm) continue;

    FlightRecord r;
    const char *icao = state[0] | "UNK";
    strncpy(r.icao, icao, sizeof(r.icao) - 1);
    r.icao[sizeof(r.icao) - 1] = '\0';

    String cs = String(state[1] | "");
    cs.trim();
    if (cs.length() == 0) cs = String(r.icao);
    strncpy(r.callsign, cs.c_str(), sizeof(r.callsign) - 1);
    r.callsign[sizeof(r.callsign) - 1] = '\0';

    const char *ctry = state[2] | "";
    strncpy(r.country, ctry, sizeof(r.country) - 1);
    r.country[sizeof(r.country) - 1] = '\0';

    r.lat       = lat;
    r.lon       = lon;
    r.alt_m     = state[7].isNull()  ? NAN  : state[7].as<float>();
    r.on_ground = state[8].isNull()  ? false : state[8].as<bool>();
    r.vel_ms    = state[9].isNull()  ? 0.0f : state[9].as<float>();
    r.track     = state[10].isNull() ? 0.0f : state[10].as<float>();
    r.dist_km   = dist;
    r.bearing   = calcBearing(userLat, userLon, lat, lon);

    r.origin[0] = '\0';
    r.dest[0]   = '\0';
    r.aircraft[0] = '\0';
    r.airline[0]  = '\0';
    r.route_fetched = false;

    insertSorted(r);
  }

  Serial.printf("[OpenSky] Processed %d flights in range\n", flightCount);

  // Auto-fetch routes for the closest 4 flights
  int lookupLimit = min(flightCount, 4);
  for (int i = 0; i < lookupLimit; i++) {
    fetchRoute(i);
  }

  return true;
}
