#include "PortraitApp.h"
#include "images.h"

PortraitApp::PortraitApp() {
  images[0] = dandelion;
  images[1] = plane;
  images[2] = flower;
  images[3] = cat;
  images[4] = building;
}

void PortraitApp::setup() {
  DisplayManager::clearScreen(TFT_BLACK);
  DisplayManager::getTft().setSwapBytes(true);
  DisplayManager::getTft().pushImage(0, 0, 240, 240, images[currentImage]);
  lastSwitch = millis();
}

void PortraitApp::update() {
  if (millis() - lastSwitch > switchInterval) {
    currentImage = (currentImage + 1) % 5;
    DisplayManager::clearScreen(TFT_BLACK);
    DisplayManager::getTft().pushImage(0, 0, 240, 240, images[currentImage]);
    lastSwitch = millis();
  }
}
