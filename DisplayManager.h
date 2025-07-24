#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>

class DisplayManager {
public:
  static const uint16_t SCREEN_WIDTH = 240;
  static const uint16_t SCREEN_HEIGHT = 240;

  static const uint16_t GAME_HEIGHT = 180;

  static TFT_eSPI& getTft();

  static void init();
  static void clearScreen(uint16_t color = TFT_BLACK);

private:
  static TFT_eSPI tftInstance;
};

#endif