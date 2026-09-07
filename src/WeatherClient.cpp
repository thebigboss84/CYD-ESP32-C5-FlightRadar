#include "WeatherClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherRecord WeatherClient::weather = { false };

const char *WeatherClient::getConditionText(int code) {
  switch (code) {
    case 0:  return "Clear Sky";
    case 1:  return "Mainly Clear";
    case 2:  return "Partly Cloudy";
    case 3:  return "Overcast";
    case 45: case 48: return "Foggy";
    case 51: case 53: case 55: return "Drizzle";
    case 61: case 63: return "Rain";
    case 65: return "Heavy Rain";
    case 71: case 73: return "Snow Fall";
    case 75: return "Heavy Snow";
    case 77: return "Snow Grains";
    case 80: case 81: return "Rain Showers";
    case 82: return "Violent Showers";
    case 85: case 86: return "Snow Showers";
    case 95: return "Thunderstorm";
    case 96: case 99: return "Storm w/ Hail";
    default: return "Variable";
  }
}

const WeatherRecord &WeatherClient::getData() {
  return weather;
}

bool WeatherClient::fetch(float lat, float lon) {
  char url[512];
  snprintf(url, sizeof(url),
    "https://api.open-meteo.com/v1/forecast"
    "?latitude=%.4f&longitude=%.4f"
    "&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m,wind_direction_10m"
    "&daily=weather_code,temperature_2m_max,temperature_2m_min"
    "&timezone=auto",
    lat, lon);

  Serial.printf("[Weather] Fetching: %s\n", url);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(12000);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[Weather] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);

  if (err) {
    Serial.printf("[Weather] JSON parse failed: %s\n", err.c_str());
    return false;
  }

  JsonObject current = doc["current"];
  weather.temp_c         = current["temperature_2m"] | 0.0f;
  weather.feels_like_c   = current["apparent_temperature"] | weather.temp_c;
  weather.humidity       = current["relative_humidity_2m"] | 0;
  weather.wind_speed_kmh = current["wind_speed_10m"] | 0.0f;
  weather.wind_dir_deg   = current["wind_direction_10m"] | 0.0f;
  weather.weather_code   = current["weather_code"] | 0;

  JsonObject daily = doc["daily"];
  JsonArray dCodes = daily["weather_code"];
  JsonArray dMax   = daily["temperature_2m_max"];
  JsonArray dMin   = daily["temperature_2m_min"];
  JsonArray dTime  = daily["time"];

  for (int i = 0; i < 4; i++) {
    weather.daily_code[i] = dCodes[i] | 0;
    weather.daily_max[i]  = dMax[i]   | 0.0f;
    weather.daily_min[i]  = dMin[i]   | 0.0f;
    const char *dt = dTime[i] | "----";
    if (strlen(dt) >= 10) {
      strncpy(weather.daily_day[i], dt + 8, 2);
      weather.daily_day[i][2] = '\0';
    } else {
      snprintf(weather.daily_day[i], sizeof(weather.daily_day[i]), "+%d", i);
    }
  }

  weather.valid = true;
  Serial.printf("[Weather] Updated: %.1fC, Code: %d, Hum: %d%%\n",
                weather.temp_c, weather.weather_code, weather.humidity);
  return true;
}
