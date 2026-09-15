#include "SpeakerAlert.h"
#include "AppConfig.h"
#include <string.h>

SpeakerAlert::Pattern SpeakerAlert::activePattern = SpeakerAlert::PATTERN_NONE;
uint8_t SpeakerAlert::step = 0;
unsigned long SpeakerAlert::stepStarted = 0;
char SpeakerAlert::announcedQuakeId[16] = {};
int64_t SpeakerAlert::announcedCountdownEpoch = 0;
int64_t SpeakerAlert::announcedLiftoffEpoch = 0;

void SpeakerAlert::begin() {
  ledcAttach(SPEAKER_PIN, 2000, 8);
  ledcWriteTone(SPEAKER_PIN, 0);
  Serial.printf("[SpeakerAlert] Piezo speaker initialized on GPIO %d\n", SPEAKER_PIN);
}

void SpeakerAlert::start(Pattern pattern) {
  activePattern = pattern;
  step = 0;
  stepStarted = 0;
}

void SpeakerAlert::stop() {
  ledcWriteTone(SPEAKER_PIN, 0);
  activePattern = PATTERN_NONE;
}

void SpeakerAlert::playStep(uint16_t frequency, uint16_t durationMs, uint16_t silenceMs) {
  unsigned long now = millis();
  unsigned long totalMs = (unsigned long)durationMs + silenceMs;
  unsigned long elapsed = now - stepStarted;

  if (elapsed < durationMs) {
    ledcWriteTone(SPEAKER_PIN, frequency);
  } else if (elapsed < totalMs) {
    ledcWriteTone(SPEAKER_PIN, 0);
  } else {
    step++;
    stepStarted = now;
  }
}

void SpeakerAlert::update(bool seismicAlert, const char *seismicId,
                          bool spaceXCountdown, bool spaceXLiftoff,
                          int64_t launchEpochUtc) {
  if (activePattern != PATTERN_NONE) {
    if (stepStarted == 0) stepStarted = millis();

    if (activePattern == PATTERN_QUAKE) {
      const uint16_t notes[] = { 660, 880, 660, 880, 660, 880 };
      playStep(notes[step], 140, 90);
      if (step >= 6) stop();
    } else if (activePattern == PATTERN_COUNTDOWN) {
      const uint16_t notes[] = { 1047, 1319 };
      playStep(notes[step], 110, 80);
      if (step >= 2) stop();
    } else {
      const uint16_t notes[] = { 784, 1047, 1568 };
      playStep(notes[step], 190, 70);
      if (step >= 3) stop();
    }
    return;
  }

  // Seismic warnings take precedence. One pattern is played for each USGS event.
  if (seismicAlert && seismicId && seismicId[0] && strcmp(seismicId, announcedQuakeId) != 0) {
    strncpy(announcedQuakeId, seismicId, sizeof(announcedQuakeId) - 1);
    announcedQuakeId[sizeof(announcedQuakeId) - 1] = '\0';
    start(PATTERN_QUAKE);
  } else if (spaceXLiftoff && launchEpochUtc > 0 && launchEpochUtc != announcedLiftoffEpoch) {
    announcedLiftoffEpoch = launchEpochUtc;
    start(PATTERN_LIFTOFF);
  } else if (spaceXCountdown && launchEpochUtc > 0 && launchEpochUtc != announcedCountdownEpoch) {
    announcedCountdownEpoch = launchEpochUtc;
    start(PATTERN_COUNTDOWN);
  }
}
