#pragma once

#include "Global.h"
#include <WiFi.h>

class ClockApp {
public:
  ClockApp();
  ~ClockApp();

  void setup();
  void update();
private:
  TFT_eSPI& tft;
  TFT_eSprite backgroundSpr;
  TFT_eSprite landscapeSpr;
  TFT_eSprite iconSpr;

  const byte ICON_WIDTH = 32;
  const byte ICON_HEIGHT = 32;
  uint16_t colorScreen;
  uint16_t* sunSprites[2];
  uint8_t frame = 0;

  const char* ssid = "SantiagoJGG";
  const char* password = "SANtiago1016*";
  const char* ntpServer = "pool.ntp.org";
  long gmtOffset_sec = -18000;  // GTM-5 (Colombia)
  int daylightOffset_sec = 0;
  struct tm timeinfo;
  static struct tm lastSync;

  const char* daysOfTheWeekES[7] = {
    "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"
  };
  const char* monthsES[12] = {
    "enero", "febrero", "marzo", "abril", "mayo", "junio",
    "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"
  };

  static void notification(struct timeval* tv);
  void makeConnection();
  void setupScreen();
  void drawAnimation();
  void showLoadingScreen();
  String buildDateES();
  void buildHourES();

  uint16_t interpolateColor(float t, uint16_t c1, uint16_t c2);

  WiFiServer _server;
  static const char PAGE_MAIN[] PROGMEM;  // HTML template for clock configuration
  void handleWebServer();                 // Processes incoming client requests
  void processParams(String req);         // Extracts and processes GET parameters
  String urlDecode(String input);
};
