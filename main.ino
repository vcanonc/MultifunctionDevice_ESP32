#include "Global.h"

#include "FlappyBirdApp.h"
#include "PortraitApp.h"
#include "ClockApp.h"

#include "logo.h"
#include "icons.h"

#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

TFT_eSPI& tft = DisplayManager::getTft();

struct MenuItem {
  const char* label;
  const unsigned short* icon;
};

const uint8_t MENU_SIZE = 4;
const MenuItem MenuItems[MENU_SIZE] = {
  { "Reloj", icon_clock },
  { "Galeria", icon_gallery },
  { "Flappy Bird", icon_chick },
  { "Musica", icon_music }
};

static uint8_t selectedIndex = 0;

// Paleta de colores 1
// uint16_t screenColor = tft.color565(10, 40, 60);
// uint16_t selectorColor = tft.color565(80, 240, 255);
// uint16_t textColor = tft.color565(220, 220, 220);
// uint16_t selectedTextColor = tft.color565(200, 100, 200);

// Paleta de colores 2
uint16_t screenColor = tft.color565(18, 18, 18);
uint16_t selectorColor = tft.color565(0, 150, 255);
uint16_t textColor = tft.color565(240, 240, 240);
uint16_t selectedTextColor = tft.color565(240, 240, 240);

// Paleta de colores 3
// uint16_t screenColor = tft.color565(24, 10, 48);
// uint16_t selectorColor = tft.color565(0, 120, 255);
// uint16_t textColor = tft.color565(230, 220, 255);
// uint16_t selectedTextColor = tft.color565(160, 200, 255);

// Paleta de colores 4
// uint16_t screenColor = tft.color565(15, 15, 35);
// uint16_t selectorColor = tft.color565(130, 0, 200);
// uint16_t textColor = tft.color565(180, 180, 255);
// uint16_t selectedTextColor = tft.color565(255, 255, 255);

void setup() {
  Serial.begin(115200);
  DisplayManager::init();

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  showWelcome();
  delay(2500);

  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.sample_rate = 44100;
  cfg.bits_per_sample = 16;
  cfg.channels = 2;   // Stereo
  cfg.pin_bck = 12;   // BCLK
  cfg.pin_ws = 13;    // LRCK / LRC
  cfg.pin_data = 14;  // DIN
  i2s.begin(cfg);
  delay(500);
  drawMenu();
}

void loop() {
  // printMemoryStats();
  menuLoop();
}

void showWelcome() {
  tft.pushImage(50, 25, 140, 140, logo);
  tft.drawCentreString("VAAS Instruments", DisplayManager::SCREEN_WIDTH / 2, 180, 4);
  tft.drawCentreString("ECHO v.1.2.", DisplayManager::SCREEN_WIDTH / 2, 215, 2);
}

void drawMenu() {
  tft.fillScreen(screenColor);

  for (uint8_t i = 0; i < MENU_SIZE; i++) {
    int x = 20;
    int y = 15 + i * 55;

    if (i == selectedIndex) {
      tft.fillRoundRect(x - 10, y - 5, 220, 50, 6, selectorColor);
    }

    tft.pushImage(x, y, 40, 40, MenuItems[i].icon, TFT_BLACK);

    tft.setTextColor(i == selectedIndex ? selectedTextColor : textColor, i == selectedIndex ? selectorColor : screenColor);
    tft.setTextSize(1);
    tft.drawString(MenuItems[i].label, x + 60, y + 9, 4);
  }
}

void menuLoop() {
  static unsigned long lastDebounceTime1 = 0;
  static unsigned long lastDebounceTime2 = 0;
  static bool lastButton1State = HIGH;
  static bool lastButton2State = HIGH;
  const unsigned long debounceDelay = 180;
  unsigned long currentTime = millis();

  if (digitalRead(BUTTON1_PIN) == LOW && lastButton1State == HIGH && currentTime - lastDebounceTime1 > debounceDelay) {
    selectedIndex = (selectedIndex + 1) % MENU_SIZE;
    drawMenu();
    lastDebounceTime1 = currentTime;
  }
  lastButton1State = digitalRead(BUTTON1_PIN);

  if (digitalRead(BUTTON2_PIN) == LOW && lastButton2State == HIGH && currentTime - lastDebounceTime2 > debounceDelay) {
    switch (selectedIndex) {
      case 0:
        startClockApp();
        break;
      case 1:
        startPortraitApp();
        break;
      case 2:
        startFlappyBirdApp();
        break;
      case 3:
        Serial.println("BLT_Start");
        startBluetoothSPK();
        Serial.println("BLT_end");
        break;
    }
    drawMenu();
    lastDebounceTime2 = currentTime;
  }
  lastButton2State = digitalRead(BUTTON2_PIN);
}

void startFlappyBirdApp() {
  FlappyBirdApp app;
  app.setup();
  while (digitalRead(BUTTON1_PIN) != LOW) {
    app.update();
    delay(10);
  }
  delay(300);
}

void startPortraitApp() {
  PortraitApp app;
  app.setup();
  while (digitalRead(BUTTON1_PIN) != LOW) {
    app.update();
    delay(10);
  }
  delay(300);
}

void startClockApp() {
  ClockApp app;
  app.setup();
  while (digitalRead(BUTTON1_PIN) != LOW) {
    app.update();
  }
  delay(300);
}

void startBluetoothSPK() {
  Serial.println("Iniciando Bluetooth A2DP...");

  // Iniciar A2DP
  a2dp_sink.start("BT_speaker");
  tft.fillScreen(screenColor);
  tft.setTextColor(selectedTextColor);
  tft.drawCentreString("Bluetooth Activado", DisplayManager::SCREEN_WIDTH / 2, 100, 2);
  tft.drawCentreString("Presiona para salir", DisplayManager::SCREEN_WIDTH / 2, 140, 2);

  while (digitalRead(BUTTON1_PIN) != LOW) {
    delay(10);
  }
  delay(300);

  Serial.println("Apagando Bluetooth...");

  // Finalizar A2DP
  a2dp_sink.end();

  // Apagar el stack Bluetooth completamente
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  // esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); // opcional pero recomendable si no usarás BT de nuevo

  Serial.println("Bluetooth desactivado.");
}

void printMemoryStats() {
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print("  |  Min Free Heap: ");
  Serial.print(ESP.getMinFreeHeap());
  Serial.print("  |  Max Alloc: ");
  Serial.println(ESP.getMaxAllocHeap());
}
