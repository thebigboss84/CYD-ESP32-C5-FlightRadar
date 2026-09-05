#include "SpaceXClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

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

bool SpaceXClient::fetch() {
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

  const char *pName = launch["pad"]["name"] | "Cape Canaveral";
  strncpy(spaceX.pad_name, pName, sizeof(spaceX.pad_name) - 1);
  spaceX.pad_name[sizeof(spaceX.pad_name) - 1] = '\0';

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
  Serial.printf("[SpaceX] Launch: %s on %s (NET: %s)\n",
                spaceX.mission_name, spaceX.rocket_name, spaceX.net_iso);
  return true;
}
