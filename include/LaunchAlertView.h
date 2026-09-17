#pragma once

#include <Arduino.h>
#include "AppConfig.h"

class LaunchAlertView {
public:
  static void draw(const SpaceXRecord &launch, int64_t secondsToLaunch);
  static bool isCloseTouched(int tx, int ty);
};
