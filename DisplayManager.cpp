#include "DisplayManager.h"

TFT_eSPI DisplayManager::tftInstance = TFT_eSPI();

TFT_eSPI& DisplayManager::getTft() {
  return tftInstance;
}

void DisplayManager::init() {
  tftInstance.init();
  tftInstance.setRotation(0);
  tftInstance.setSwapBytes(true);
  tftInstance.fillScreen(TFT_BLACK);
}

void DisplayManager::clearScreen(uint16_t color) {
  tftInstance.fillScreen(color);
}
