#pragma once

#include <Arduino.h>

class SpeakerAlert {
public:
  static void begin();
  static void update(bool seismicAlert, const char *seismicId,
                     bool spaceXCountdown, bool spaceXLiftoff,
                     int64_t launchEpochUtc);

private:
  enum Pattern { PATTERN_NONE, PATTERN_QUAKE, PATTERN_COUNTDOWN, PATTERN_LIFTOFF };

  static Pattern activePattern;
  static uint8_t step;
  static unsigned long stepStarted;
  static char announcedQuakeId[16];
  static int64_t announcedCountdownEpoch;
  static int64_t announcedLiftoffEpoch;

  static void start(Pattern pattern);
  static void stop();
  static void playStep(uint16_t frequency, uint16_t durationMs, uint16_t silenceMs);
};
