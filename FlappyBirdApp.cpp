#include "FlappyBirdApp.h"
#include "gameSprites.h"


FlappyBirdApp::FlappyBirdApp()
  : tft(DisplayManager::getTft()),
    backgroundSpr(&tft),
    flappySpr(&tft) {
  birdSprites[0] = bird0;
  birdSprites[1] = bird2;
  birdSprites[2] = bird1;
}

FlappyBirdApp::~FlappyBirdApp() {
  flappySpr.deleteSprite();
  backgroundSpr.deleteSprite();
}

void FlappyBirdApp::setup() {
  tft.fillScreen(TFT_BLACK);
  highScore = readHighScore();
  delay(50);

  backgroundSpr.createSprite(DisplayManager::SCREEN_WIDTH, DisplayManager::GAME_HEIGHT);
  backgroundSpr.setSwapBytes(true);
  backgroundSpr.setTextColor(TFT_WHITE, TFT_BLACK);
  flappySpr.createSprite(32, 23);

  colorPipe = tft.color565(143, 255, 38);

  randomSeed(analogRead(0));
}

void FlappyBirdApp::update() {
  if (gameState == 0) {
    backgroundSpr.pushImage(0, 0, DisplayManager::SCREEN_WIDTH, DisplayManager::GAME_HEIGHT, bg_img);

    if (digitalRead(BUTTON2_PIN) == LOW) {
      momentum = -4;
    }

    momentum += 1;
    birdY += momentum;

    if (birdY < 0) birdY = 0;
    if (birdY > DisplayManager::GAME_HEIGHT - 23) {
      birdY = DisplayManager::GAME_HEIGHT - 23;
      momentum = -2;
    }

    if (momentum < 0) {
      uint8_t frame = random(0, 3);
      flappySpr.pushImage(0, 0, 32, 23, birdSprites[frame]);
    } else {
      flappySpr.pushImage(0, 0, 32, 23, birdSprites[2]);
    }

    flappySpr.pushToSprite(&backgroundSpr, 40, birdY, TFT_BLACK);

    for (uint8_t i = 0; i < 2; i++) {
      backgroundSpr.fillRect(wallX[i], 0, wallWidth, wallY[i], colorPipe);
      backgroundSpr.drawRect(wallX[i], 0, wallWidth, wallY[i], TFT_BLACK);
      backgroundSpr.fillRoundRect(wallX[i] - 3, wallY[i] - 20, wallWidth + 6, 20, 4, colorPipe);
      backgroundSpr.drawRoundRect(wallX[i] - 3, wallY[i] - 20, wallWidth + 6, 20, 4, TFT_BLACK);
      backgroundSpr.fillRect(wallX[i], wallY[i] + wallGap, wallWidth, DisplayManager::GAME_HEIGHT - wallY[i] + wallGap, colorPipe);
      backgroundSpr.drawRect(wallX[i], wallY[i] + wallGap, wallWidth, DisplayManager::GAME_HEIGHT - wallY[i] + wallGap, TFT_BLACK);
      backgroundSpr.fillRoundRect(wallX[i] - 3, wallY[i] + wallGap, wallWidth + 6, 20, 4, colorPipe);
      backgroundSpr.drawRoundRect(wallX[i] - 3, wallY[i] + wallGap, wallWidth + 6, 20, 4, TFT_BLACK);

      if (wallX[i] < 0) {
        wallY[i] = random(0, DisplayManager::GAME_HEIGHT - wallGap - 20);
        wallX[i] = DisplayManager::SCREEN_WIDTH;
      }

      if (wallX[i] == birdX) {
        score++;
        if (score > highScore) {
          highScore = score;
          updateHighScore(score);
        }
      }

      const uint8_t bird_collider_padding = 1;
      if (
        (birdX + 32 / 2 > wallX[i] && birdX < wallX[i] + wallWidth) && (birdY < wallY[i] || birdY + 23 - 10 > wallY[i] + wallGap)) {
        delay(1000);
        backgroundSpr.fillSprite(TFT_BLACK);
        backgroundSpr.drawString("GAME OVER", 50, 60, 4);
        refreshScreen();
        gameState = 1;
        delay(2000);
      }

      wallX[i] -= 4;
    }

    backgroundSpr.drawString(String(score), 20, 0, 2);
    refreshScreen();
    delay(60);
  } else {
    backgroundSpr.fillSprite(TFT_BLACK);
    backgroundSpr.drawString("FLAPPY BIRD", 50, 10, 4);
    backgroundSpr.drawString("Presiona para empezar", 55, 40, 2);
    backgroundSpr.drawString("Puntaje: " + String(score), 90, 60, 2);
    backgroundSpr.drawString("Record: " + String(highScore), 90, 80, 2);
    refreshScreen();

    while (digitalRead(BUTTON2_PIN) == LOW) {
      if (digitalRead(BUTTON1_PIN) == LOW)
        break;
    }

    birdY = DisplayManager::GAME_HEIGHT / 2;
    momentum = -4;
    wallX[0] = DisplayManager::SCREEN_WIDTH;
    wallY[0] = DisplayManager::GAME_HEIGHT / 2 - wallGap / 2;
    wallX[1] = DisplayManager::SCREEN_WIDTH + DisplayManager::SCREEN_WIDTH / 2;
    wallY[1] = DisplayManager::GAME_HEIGHT / 2 - wallGap / 1;
    score = 0;

    while (digitalRead(BUTTON2_PIN) == HIGH) {
      if (digitalRead(BUTTON1_PIN) == LOW)
        break;
    }
    gameState = 0;
  }
}

void FlappyBirdApp::refreshScreen() {
  backgroundSpr.pushSprite(0, 30);
}

uint16_t FlappyBirdApp::readHighScore() {
  preferences.begin("flappy-bird", true);
  uint16_t value = (uint16_t)preferences.getUInt("highScore", 0);
  preferences.end();
  return value;
}

void FlappyBirdApp::updateHighScore(uint16_t newScore) {
  preferences.begin("flappy-bird", false);
  preferences.putUInt("highScore", newScore);
  preferences.end();
}
