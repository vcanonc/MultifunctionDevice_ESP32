#pragma once

#include "Global.h"
#include <Preferences.h>

class FlappyBirdApp {
public:
  FlappyBirdApp();
  ~FlappyBirdApp();

  void setup();
  void update();

private:
  TFT_eSPI& tft;
  Preferences preferences;

  TFT_eSprite backgroundSpr;
  TFT_eSprite flappySpr;

  const unsigned short* birdSprites[4];

  uint8_t gameState = 1;
  uint16_t score = 0;
  uint16_t highScore = 0;
  int16_t birdX = 40;
  int16_t birdY = 80;
  int8_t momentum = 0;

  int16_t wallX[2];
  int16_t wallY[2];
  const uint8_t wallGap = 48;
  const uint8_t wallWidth = 37;
  uint16_t colorPipe;

  void refreshScreen();
  uint16_t readHighScore();
  void updateHighScore(uint16_t newScore);
};
