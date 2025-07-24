#pragma once

#include "Global.h"

class PortraitApp {
public:
  PortraitApp();
  void setup();
  void update();

private:
  const uint16_t* images[5];
  uint8_t currentImage = 0;
  unsigned long lastSwitch = 0;
  const uint32_t switchInterval = 5000;  // 5 seconds
};
