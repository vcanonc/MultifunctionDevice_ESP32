#include "ClockApp.h"
#include "esp_sntp.h"
#include "esp32-hal-gpio.h"
#include "TFT_eSPI.h"
#include <WiFi.h>
#include "time.h"
#include "icons.h"

struct tm ClockApp::lastSync = {};

void ClockApp::notification(struct timeval* tv) {
  getLocalTime(&lastSync);
}

const char ClockApp::PAGE_MAIN[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuración del Reloj</title>
    <style>
        body {
            font-family: sans-serif;
            background-color: #111;
            color: #eee;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 2rem;
        }
        .container {
            background: #222;
            padding: 2rem;
            border-radius: 1rem;
            max-width: 400px;
            width: 100%;
        }
        label, select, input {
            display: block;
            width: 100%;
            margin-bottom: 1rem;
        }
        button {
            padding: 1rem;
            width: 100%;
            font-weight: bold;
            background: #4CAF50;
            border: none;
            color: white;
            border-radius: 0.5rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Configuración del Reloj</h2>
        <form action="/set" method="get">
            <label for="gmt">Zona Horaria (GMT)</label>
            <select name="gmt" id="gmt">
                <option value="-13" selected>Seleccione un huso horario</option>
                <option value="-12">GMT-12</option>
                <option value="-11">GMT-11</option>
                <option value="-10">GMT-10</option>
                <option value="-9.5">GMT-9:30</option>
                <option value="-9">GMT-9</option>
                <option value="-8">GMT-8</option>
                <option value="-7">GMT-7</option>
                <option value="-6">GMT-6</option>
                <option value="-5.5">GMT-5:30</option>
                <option value="-5">GMT-5</option>
                <option value="-4.5">GMT-4:30</option>
                <option value="-4">GMT-4</option>
                <option value="-3.5">GMT-3:30</option>
                <option value="-3">GMT-3</option>
                <option value="-2">GMT-2</option>
                <option value="-1">GMT-1</option>
                <option value="0">GMT</option>
                <option value="1">GMT+1</option>
                <option value="2">GMT+2</option>
                <option value="3">GMT+3</option>
                <option value="3.5">GMT+3:30</option>
                <option value="4">GMT+4</option>
                <option value="4.5">GMT+4:30</option>
                <option value="5">GMT+5</option>
                <option value="5.5">GMT+5:30</option>
                <option value="5.75">GMT+5:45</option>
                <option value="6">GMT+6</option>
                <option value="6.5">GMT+6:30</option>
                <option value="7">GMT+7</option>
                <option value="8">GMT+8</option>
                <option value="8.75">GMT+8:45</option>
                <option value="9">GMT+9</option>
                <option value="9.5">GMT+9:30</option>
                <option value="10">GMT+10</option>
                <option value="10.5">GMT+10:30</option>
                <option value="11">GMT+11</option>
                <option value="12">GMT+12</option>
                <option value="12.75">GMT+12:45</option>
                <option value="13">GMT+13</option>
                <option value="14">GMT+14</option>
            </select>

            <label for="hora">Hora manual (HH:MM:SS)</label>
            <input type="text" id="hora" name="hora" placeholder="14:30:00">

            <button type="submit">Actualizar Hora</button>
        </form>
        <p>Hora actual: %HORA%</p>
    </div>
</body>
</html>
)rawliteral";

ClockApp::ClockApp()
  : tft(DisplayManager::getTft()),
    backgroundSpr(&tft),
    landscapeSpr(&tft),
    iconSpr(&tft),
    _server(80) {
  sunSprites[0] = (uint16_t*)icon_sun0;
  sunSprites[1] = (uint16_t*)icon_sun1;
  colorScreen = TFT_BLACK;
}

ClockApp::~ClockApp() {
  backgroundSpr.deleteSprite();
  landscapeSpr.deleteSprite();
  iconSpr.deleteSprite();
  _server.stop();
}

void ClockApp::setup() {
  setupScreen();
  makeConnection();
  randomSeed(analogRead(0));
}

void ClockApp::update() {
  static int step = 0;
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();

  switch (step) {
    case 0:
      {
        handleWebServer();
        getLocalTime(&timeinfo);

        String date = buildDateES();

        char hour[10];
        strftime(hour, sizeof(hour), "%H:%M:%S", &timeinfo);

        // Show in the screen
        backgroundSpr.fillSprite(colorScreen);
        backgroundSpr.drawCentreString(hour, backgroundSpr.width() / 2, 5, 7);
        backgroundSpr.drawCentreString(daysOfTheWeekES[timeinfo.tm_wday], backgroundSpr.width() / 2, 60, 4);
        backgroundSpr.drawCentreString(date, backgroundSpr.width() / 2, 85, 4);

        drawAnimation();
        landscapeSpr.pushSprite(10, 10);

        backgroundSpr.pushSprite(10, 121);

        lastMillis = currentMillis;
        step = 1;
        break;
      }

    case 1:
      if (currentMillis - lastMillis >= 1000) {
        step = 0;  // permitir nueva actualización después de 1s
      }
      break;
  }
}

void ClockApp::makeConnection() {
  backgroundSpr.drawCentreString("Cargando...", backgroundSpr.width() / 2, 0, 4);
  backgroundSpr.pushSprite(10, 121);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (digitalRead(BUTTON1_PIN) == LOW)
      break;
  }

  Serial.println("\nWiFi conectado.");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  backgroundSpr.drawCentreString(WiFi.localIP().toString(), backgroundSpr.width() / 2, 30, 4);
  backgroundSpr.pushSprite(10, 121);
  delay(1000);

  _server.begin();

  sntp_set_sync_interval(604800000UL);  // 7 days
  sntp_set_time_sync_notification_cb(notification);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Esperando sincronización NTP...");
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    if (digitalRead(BUTTON1_PIN) == LOW)
      break;
  }
  Serial.println("\nHora sincronizada.");
}

String ClockApp::buildDateES() {
  return String(timeinfo.tm_mday) + " de " + String(monthsES[timeinfo.tm_mon]) + " de " + String(1900 + timeinfo.tm_year);
}

void ClockApp::handleWebServer() {
  WiFiClient client = _server.available();
  if (client) {
    Serial.println("New client connected (Clock Module Web Server).");
    String currentLine = "";
    String req = "";

    // Read the client request
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        if (c == '\n') {
          // End of HTTP header
          if (currentLine.length() == 0) {
            // Process parameters if the GET URL contains '/set'
            if (req.startsWith("GET /set")) {
              processParams(req);
            }

            // Get current time as string
            char currentTime[64];
            getLocalTime(&timeinfo);
            strftime(currentTime, sizeof(currentTime), "%H:%M:%S", &timeinfo);

            // Create page based on stored HTML template
            String page = FPSTR(PAGE_MAIN);
            page.replace("%HORA%", currentTime);

            // Send HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.print(page);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected from Clock Module Web Server.");
  }
}

void ClockApp::processParams(String req) {
  int gmtIndex = req.indexOf("gmt=");
  int timeIndex = req.indexOf("hora=");

  if (gmtIndex < 0 && timeIndex < 0) return;

  // --- Procesar GMT si está definido ---
  if (gmtIndex >= 0) {
    int gmtEnd = req.indexOf('&', gmtIndex);
    if (gmtEnd == -1) gmtEnd = req.indexOf(' ', gmtIndex);
    if (gmtEnd == -1) gmtEnd = req.length();

    String gmtStr = req.substring(gmtIndex + 4, gmtEnd);
    gmtStr.replace(",", ".");

    float gmtValue = gmtStr.toFloat();
    if (gmtValue != -13.0f) {
      gmtOffset_sec = (long)(gmtValue * 3600);

      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      if (timeIndex < 0) {
        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);         // Reactivar sincronización periódica
        sntp_set_time_sync_notification_cb(notification);  // Restaurar callback
        sntp_servermode_dhcp(1);                           // Permitir actualización desde DHCP
        Serial.println("SNTP re-enabled after GMT set.");
      }

      Serial.printf("GMT updated: %.2f → %ld seconds offset\n", gmtValue, gmtOffset_sec);
    }
  }

  // --- Procesar hora manual ---
  if (timeIndex >= 0) {
    int timeEnd = req.indexOf('&', timeIndex);
    if (timeEnd == -1) timeEnd = req.indexOf(' ', timeIndex);
    if (timeEnd == -1) timeEnd = req.length();

    String timeStr = urlDecode(req.substring(timeIndex + 5, timeEnd));

    int h, m, s;
    if (sscanf(timeStr.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
      time_t now;
      time(&now);
      struct tm t;
      gmtime_r(&now, &t);

      t.tm_hour = h;
      t.tm_min = m;
      t.tm_sec = s;

      time_t manualEpoch = mktime(&t);
      struct timeval tv = { .tv_sec = manualEpoch };
      settimeofday(&tv, nullptr);

      sntp_servermode_dhcp(0);                      // Evita que DHCP sobrescriba servidores NTP
      sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);     // Cambia a sincronización inmediata (no periódica)
      sntp_set_time_sync_notification_cb(nullptr);  // Elimina el callback de notificación

      Serial.printf("Manual time set to: %02d:%02d:%02d\n", h, m, s);
    } else {
      Serial.printf("Invalid time format: [%s]\n", timeStr.c_str());
    }
  }
}

void ClockApp::setupScreen() {
  tft.fillScreen(colorScreen);
  delay(50);

  backgroundSpr.createSprite(DisplayManager::SCREEN_WIDTH - 20, 110);
  backgroundSpr.setSwapBytes(true);
  backgroundSpr.setTextColor(TFT_WHITE);
  backgroundSpr.fillSprite(colorScreen);

  landscapeSpr.createSprite(DisplayManager::SCREEN_WIDTH - 20, 110);
  landscapeSpr.setSwapBytes(true);
  landscapeSpr.fillSprite(colorScreen);

  iconSpr.createSprite(ICON_WIDTH, ICON_HEIGHT);

  landscapeSpr.pushSprite(10, 10);
  backgroundSpr.pushSprite(10, 121);
}

uint16_t ClockApp::interpolateColor(float t, uint16_t c1, uint16_t c2) {
  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  float r1 = (c1 >> 11) & 0x1F;
  float g1 = (c1 >> 5) & 0x3F;
  float b1 = c1 & 0x1F;

  float r2 = (c2 >> 11) & 0x1F;
  float g2 = (c2 >> 5) & 0x3F;
  float b2 = c2 & 0x1F;

  int r = round(r1 + (r2 - r1) * t);
  int g = round(g1 + (g2 - g1) * t);
  int b = round(b1 + (b2 - b1) * t);

  // Clamp por seguridad
  r = constrain(r, 0, 31);
  g = constrain(g, 0, 63);
  b = constrain(b, 0, 31);

  return (r << 11) | (g << 5) | b;
}


void ClockApp::drawAnimation() {

  uint16_t skyColor;

  // Cálculo de hora actual
  float sunrise = 5.833;  // 5:50
  float sunset = 18.2;    // 18:12
  float hour = timeinfo.tm_hour + timeinfo.tm_min / 60.0f;
  float angle;
  int centerX = landscapeSpr.width() / 2;
  int centerY = landscapeSpr.height();
  int radius = 80;
  float rad;

  // Determina el color de fondo del cielo dependiendo de la hora actual
  if (hour < 6.0) {
    // 5:00 – 6:00: negro → azul oscuro
    skyColor = interpolateColor(hour - 5.0, TFT_BLACK, TFT_NAVY);
  } else if (hour < 8.0) {
    // 6:00 – 8:00: azul oscuro → celeste
    skyColor = interpolateColor((hour - 6.0) / 2.0, TFT_NAVY, TFT_SKYBLUE);
  } else if (hour < 12.0) {
    // 8:00 – 12:00: celeste → cian (máxima luz)
    skyColor = interpolateColor((hour - 8.0) / 4.0, TFT_SKYBLUE, TFT_CYAN);
  } else if (hour < 15.0) {
    // 12:00 – 15:00: cian → celeste (empieza a bajar)
    skyColor = interpolateColor((hour - 12.0) / 3.0, TFT_CYAN, TFT_SKYBLUE);
  } else if (hour < 17.0) {
    // 15:00 – 17:00: celeste → naranja (puesta de sol)
    skyColor = interpolateColor((hour - 15.0) / 2.0, TFT_SKYBLUE, TFT_ORANGE);
  } else if (hour < 18.5) {
    // 17:00 – 18:30: naranja → gris oscuro
    skyColor = interpolateColor((hour - 17.0) / 1.5, TFT_ORANGE, TFT_DARKGREY);
  } else if (hour < 19.25) {
    // 18:30 – 19:15: gris oscuro → violeta (anochecer)
    skyColor = interpolateColor((hour - 18.5) / 0.75, TFT_DARKGREY, TFT_VIOLET);
  } else if (hour < 20.0) {
    // 19:15 – 20:00: violeta → negro
    skyColor = interpolateColor((hour - 19.25) / 0.75, TFT_VIOLET, TFT_BLACK);
  } else {
    // 20:00 – 5:00: noche total
    skyColor = TFT_BLACK;
  }

  landscapeSpr.fillSprite(skyColor);

  // Horizonte
  int horizonY = landscapeSpr.height() - 1;
  landscapeSpr.fillRoundRect(0, horizonY - 2, landscapeSpr.width(), 4, 2, TFT_WHITE);

  // Semicírculo punteado visible
  for (int angle = 0; angle <= 180; angle += 6) {
    float rad = angle * DEG_TO_RAD;
    int x = centerX + radius * cos(rad);
    int y = centerY - radius * sin(rad);
    if (y >= 0 && y < landscapeSpr.height()) {
      landscapeSpr.fillCircle(x, y, 1, TFT_WHITE);
    }
  }

  if (hour >= sunrise && hour <= sunset) {
    // Día: sol de 0° a 180°
    float t = (hour - sunrise) / (sunset - sunrise);
    angle = t * 180.0f;
  } else {
    // Noche: luna de 0° a 180°
    float nightDuration = (24 - sunset) + sunrise;
    float t;
    if (hour > sunset) {
      t = (hour - sunset) / nightDuration;
    } else {
      t = (hour + (24 - sunset)) / nightDuration;
    }
    angle = t * 180.0f;
  }

  rad = angle * DEG_TO_RAD;
  int iconX = centerX + radius * cos(rad) - ICON_WIDTH / 2;
  int iconY = centerY - radius * sin(rad) - ICON_HEIGHT / 2;

  // Dibuja el sol o luna
  frame = (frame + 1) % 2;
  iconSpr.pushImage(0, 0, ICON_WIDTH, ICON_HEIGHT, hour >= sunrise && hour <= sunset ? sunSprites[frame] : (uint16_t*)icon_moon, TFT_BLACK);
  iconSpr.pushToSprite(&landscapeSpr, iconX, iconY, TFT_BLACK);
}

String ClockApp::urlDecode(String input) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = input.length();
  unsigned int i = 0;

  while (i < len) {
    char c = input.charAt(i);
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < len) {
      temp[2] = input.charAt(i + 1);
      temp[3] = input.charAt(i + 2);
      decoded += (char)strtol(temp, nullptr, 16);
      i += 2;
    } else {
      decoded += c;
    }
    ++i;
  }
  return decoded;
}
