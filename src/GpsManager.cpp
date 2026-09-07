#include "GpsManager.h"
#include <HardwareSerial.h>
#include <math.h>

GpsData GpsManager::data = { false, 0.0f, 0.0f, 0.0f, 0, 0.0f, 99.9f, 0 };
float GpsManager::lastReportedLat = 0.0f;
float GpsManager::lastReportedLon = 0.0f;
bool  GpsManager::hasReportedOnce = false;

static HardwareSerial gpsSerial(1);
static char nmeaBuf[128];
static int  nmeaIdx = 0;

void GpsManager::begin() {
  Serial.printf("[GPS] Initializing UART1 on RX=%d, TX=%d at %d baud...\n",
                GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD);
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  nmeaIdx = 0;
}

void GpsManager::update() {
  while (gpsSerial.available() > 0) {
    char c = (char)gpsSerial.read();
    if (c == '$') {
      nmeaIdx = 0;
      nmeaBuf[nmeaIdx++] = c;
    } else if (c == '\r' || c == '\n') {
      if (nmeaIdx > 6) {
        nmeaBuf[nmeaIdx] = '\0';
        parseNmeaSentence(nmeaBuf);
      }
      nmeaIdx = 0;
    } else if (nmeaIdx < (int)sizeof(nmeaBuf) - 1) {
      nmeaBuf[nmeaIdx++] = c;
    }
  }

  // If no fix received for 10 seconds, mark fix as lost
  if (data.has_fix && (millis() - data.last_fix_ms > 10000)) {
    data.has_fix = false;
    Serial.println("[GPS] Satellite fix timed out (>10s without valid fix)");
  }
}

bool GpsManager::validateChecksum(const char *sentence) {
  const char *star = strchr(sentence, '*');
  if (!star || strlen(star) < 3) return false;

  uint8_t expected = (uint8_t)strtol(star + 1, NULL, 16);
  uint8_t sum = 0;
  for (const char *p = sentence + 1; p < star; p++) {
    sum ^= (uint8_t)(*p);
  }
  return (sum == expected);
}

float GpsManager::parseNmeaCoord(const char *str, char dir) {
  if (!str || strlen(str) < 4) return 0.0f;
  float raw = atof(str);
  int deg = (int)(raw / 100.0f);
  float minutes = raw - (float)(deg * 100);
  float dec = (float)deg + (minutes / 60.0f);
  if (dir == 'S' || dir == 'W' || dir == 's' || dir == 'w') {
    dec = -dec;
  }
  return dec;
}

void GpsManager::parseNmeaSentence(const char *sentence) {
  if (!validateChecksum(sentence)) return;

  // Split comma-separated tokens
  char buf[128];
  strncpy(buf, sentence, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char *tokens[24];
  int tokenCount = 0;
  char *p = buf;
  tokens[tokenCount++] = p;
  while (*p && tokenCount < 24) {
    if (*p == ',' || *p == '*') {
      *p = '\0';
      tokens[tokenCount++] = p + 1;
    }
    p++;
  }

  if (tokenCount < 2) return;
  const char *type = tokens[0];

  // 1. $GPRMC / $GNRMC (Recommended Minimum Navigation Information)
  if (strstr(type, "RMC")) {
    // tokens[2]: status 'A' (valid) or 'V' (void)
    if (tokenCount > 7 && tokens[2][0] == 'A') {
      char dirLat = tokens[4][0];
      char dirLon = tokens[6][0];
      float lat = parseNmeaCoord(tokens[3], dirLat);
      float lon = parseNmeaCoord(tokens[5], dirLon);
      float spdKnots = (tokenCount > 7 && strlen(tokens[7]) > 0) ? atof(tokens[7]) : 0.0f;

      if (lat != 0.0f && lon != 0.0f) {
        data.lat = lat;
        data.lon = lon;
        data.speed_kmh = spdKnots * 1.852f;
        data.has_fix = true;
        data.last_fix_ms = millis();
      }
    }
  }
  // 2. $GPGGA / $GNGGA (Global Positioning System Fix Data)
  else if (strstr(type, "GGA")) {
    // tokens[6]: fix quality (0 = invalid, >0 = valid)
    if (tokenCount > 9 && atoi(tokens[6]) > 0) {
      char dirLat = tokens[3][0];
      char dirLon = tokens[5][0];
      float lat = parseNmeaCoord(tokens[2], dirLat);
      float lon = parseNmeaCoord(tokens[4], dirLon);
      int sats  = atoi(tokens[7]);
      float hdop = atof(tokens[8]);
      float alt  = (tokenCount > 9) ? atof(tokens[9]) : 0.0f;

      if (lat != 0.0f && lon != 0.0f) {
        data.lat = lat;
        data.lon = lon;
        data.satellites = sats;
        data.hdop = hdop;
        data.alt_m = alt;
        data.has_fix = true;
        data.last_fix_ms = millis();
      }
    } else if (tokenCount > 7) {
      data.satellites = atoi(tokens[7]);
    }
  }
}

bool GpsManager::hasFix() {
  return data.has_fix;
}

float GpsManager::getLat() {
  return data.lat;
}

float GpsManager::getLon() {
  return data.lon;
}

float GpsManager::getAltitudeM() {
  return data.alt_m;
}

int GpsManager::getSatellites() {
  return data.satellites;
}

float GpsManager::getSpeedKmh() {
  return data.speed_kmh;
}

float GpsManager::getHdop() {
  return data.hdop;
}

unsigned long GpsManager::getLastFixMs() {
  return data.last_fix_ms;
}

const GpsData &GpsManager::getData() {
  return data;
}

bool GpsManager::hasMovedSignificantly(float thresholdKm) {
  if (!data.has_fix) return false;
  if (!hasReportedOnce) {
    markReportedPosition();
    return true;
  }

  // Haversine distance
  float dLat = (data.lat - lastReportedLat) * (float)M_PI / 180.0f;
  float dLon = (data.lon - lastReportedLon) * (float)M_PI / 180.0f;
  float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f) +
            cosf(lastReportedLat * (float)M_PI / 180.0f) * cosf(data.lat * (float)M_PI / 180.0f) *
            sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
  float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
  float distKm = 6371.0f * c;

  if (distKm >= thresholdKm) {
    markReportedPosition();
    return true;
  }
  return false;
}

void GpsManager::markReportedPosition() {
  lastReportedLat = data.lat;
  lastReportedLon = data.lon;
  hasReportedOnce = true;
}
