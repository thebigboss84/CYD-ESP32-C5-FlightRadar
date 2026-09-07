#include "LedBeacon.h"
#include <math.h>

bool LedBeacon::emergencyActive  = false;
bool LedBeacon::seismicAlert     = false;
float LedBeacon::seismicMag      = 0.0f;
bool LedBeacon::spaceXCountdown  = false;
bool LedBeacon::spaceXLiftoff    = false;
bool LedBeacon::issPassActive    = false;
bool LedBeacon::aircraftOverhead = false;

unsigned long LedBeacon::lastTick = 0;
int  LedBeacon::strobePhase = 0;

void LedBeacon::begin() {
  pinMode(LED_PIN, OUTPUT);
  writeColor(0, 0, 0);
  Serial.printf("[LedBeacon] WS2812 RGB LED initialized on GPIO %d\n", LED_PIN);
}

void LedBeacon::writeColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(LED_PIN, r, g, b);
}

void LedBeacon::setEmergency(bool active) {
  emergencyActive = active;
}

void LedBeacon::setSeismicAlert(bool active, float mag) {
  seismicAlert = active;
  seismicMag   = mag;
}

void LedBeacon::setSpaceXState(bool countdown, bool liftoff) {
  spaceXCountdown = countdown;
  spaceXLiftoff   = liftoff;
}

void LedBeacon::setIssPass(bool active) {
  issPassActive = active;
}

void LedBeacon::setAircraftOverhead(bool active) {
  aircraftOverhead = active;
}

void LedBeacon::setOff() {
  writeColor(0, 0, 0);
}

void LedBeacon::update() {
  unsigned long now = millis();

  // 1. Highest Priority: In-flight emergency squawk 7700/7600 (Fast 10Hz Red Flash)
  if (emergencyActive) {
    if (now - lastTick >= 50) {
      lastTick = now;
      strobePhase = (strobePhase + 1) % 2;
      if (strobePhase == 0) {
        writeColor(255, 0, 0); // Bright Red
      } else {
        writeColor(0, 0, 0);
      }
    }
    return;
  }

  // 1b. Seismic Warning Alert (< 200 km, M >= 2.0 within last 90 min) - High-Intensity Warning Red Pulse
  if (seismicAlert) {
    if (now - lastTick >= 35) {
      lastTick = now;
      float phase = (float)(now % 650) / 650.0f * (float)M_PI * 2.0f;
      float bright = 0.20f + 0.80f * ((sinf(phase) + 1.0f) * 0.5f);
      uint8_t r = (uint8_t)(bright * 255.0f);
      writeColor(r, 0, 0); // Pulsing Pure Warning Red
    }
    return;
  }

  // 2. SpaceX Active Liftoff (Rapid 4Hz Amber Strobe)
  if (spaceXLiftoff) {
    if (now - lastTick >= 120) {
      lastTick = now;
      strobePhase = (strobePhase + 1) % 2;
      if (strobePhase == 0) {
        writeColor(255, 140, 0); // High-intensity Amber
      } else {
        writeColor(0, 0, 0);
      }
    }
    return;
  }

  // 3. Aircraft Overhead (< 6 km) - Real Aviation Double White Wingtip Strobe
  if (aircraftOverhead) {
    // Pattern: Flash 40ms, Off 80ms, Flash 40ms, Off 2000ms (Period = 2160ms)
    unsigned long phaseMs = now % 2200;
    if ((phaseMs >= 0 && phaseMs < 40) || (phaseMs >= 120 && phaseMs < 160)) {
      writeColor(255, 255, 255); // Brilliant White
    } else {
      writeColor(0, 0, 0);
    }
    return;
  }

  // 4. SpaceX Countdown (Pulsing high-visibility amber glow, 2-second cycle)
  if (spaceXCountdown) {
    if (now - lastTick >= 30) {
      lastTick = now;
      float phase = (float)(now % 2000) / 2000.0f * (float)M_PI * 2.0f;
      float bright = 0.25f + 0.75f * ((sinf(phase) + 1.0f) * 0.5f); // 0.25 .. 1.0
      uint8_t r = (uint8_t)(bright * 255.0f);
      uint8_t g = (uint8_t)(bright * 140.0f);
      writeColor(r, g, 0);
    }
    return;
  }

  // 5. ISS Overhead Pass (Breathing Interstellar Purple, 2.5-second cycle)
  if (issPassActive) {
    if (now - lastTick >= 30) {
      lastTick = now;
      float phase = (float)(now % 2500) / 2500.0f * (float)M_PI * 2.0f;
      float bright = 0.25f + 0.75f * ((sinf(phase) + 1.0f) * 0.5f); // 0.25 .. 1.0
      uint8_t r = (uint8_t)(bright * 220.0f);
      uint8_t b = (uint8_t)(bright * 255.0f);
      writeColor(r, 0, b);
    }
    return;
  }

  // 6. Normal Idle: Luminous tactical cyan radar glow (15x brighter, vibrant breath)
  if (now - lastTick >= 40) {
    lastTick = now;
    float phase = (float)(now % 3000) / 3000.0f * (float)M_PI * 2.0f;
    float bright = 0.20f + 0.80f * ((sinf(phase) + 1.0f) * 0.5f); // 0.20 .. 1.0
    uint8_t g = (uint8_t)(bright * 180.0f);
    uint8_t b = (uint8_t)(bright * 255.0f);
    writeColor(0, g, b);
  }
}
