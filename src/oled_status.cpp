#include "oled_status.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <esp_system.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static String lastDevice = "ULTIMESH";
static String lastConn = "USB";

void initOLED(const char* deviceName, const char* connectionType) {
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  delay(100);

  Wire.begin(4, 15);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] SSD1306 init failed");
    return;
  }

  lastDevice = deviceName;
  lastConn = connectionType;
  drawPagerScreen(deviceName, connectionType);
}

void drawTopScreen() {
  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("MONITOR");

  display.setCursor(0, 12);
  display.print("CPU: ");
  display.println("N/A");

  display.setCursor(0, 24);
  display.print("TEMP: ");
  display.println("N/A");

  display.setCursor(0, 36);
  display.print("RAM: ");
  display.print(ESP.getFreeHeap() / 1024);
  display.println(" KB");

  size_t total = SPIFFS.totalBytes();
  size_t used = SPIFFS.usedBytes();
  display.setCursor(0, 48);
  display.print("FL: ");
  display.print(used / 1024);
  display.print("/");
  display.print(total / 1024);
  display.println(" KB");

  display.display();
}

void drawPagerScreen(const char* deviceName, const char* connectionType) {
  display.setRotation(0);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ULTIMESH STANDBY");
  display.println("READY TO RECEIVE...");
  display.println("LORA LINK ACTIVE");

  // Time and date display (dummy for now)
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M %d/%m", &timeinfo);

  display.setCursor(0, 48);
  display.println(buffer);

  display.display();
}
