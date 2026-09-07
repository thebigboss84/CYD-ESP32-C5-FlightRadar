#pragma once

#include <Arduino.h>
#include "AppConfig.h"

enum BeaconState {
  BEACON_IDLE = 0,
  BEACON_AIRCRAFT_OVERHEAD,
  BEACON_ISS_PASS,
  BEACON_SPACEX_COUNTDOWN,
  BEACON_SPACEX_LIFTOFF,
  BEACON_EMERGENCY
};

class LedBeacon {
public:
  static void begin();
  static void update();

  static void setEmergency(bool active);
  static void setSeismicAlert(bool active, float mag = 0.0f);
  static void setSpaceXState(bool countdown, bool liftoff);
  static void setIssPass(bool active);
  static void setAircraftOverhead(bool active);

  static void setOff();

private:
  static bool emergencyActive;
  static bool seismicAlert;
  static float seismicMag;
  static bool spaceXCountdown;
  static bool spaceXLiftoff;
  static bool issPassActive;
  static bool aircraftOverhead;

  static unsigned long lastTick;
  static int  strobePhase;
  static void writeColor(uint8_t r, uint8_t g, uint8_t b);
};
