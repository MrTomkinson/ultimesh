#include "oled_status.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initOLED(const char* deviceName, const char* connectionType) {
  Wire.begin(4, 15);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] init fail");
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(deviceName);
  display.print("Conn: ");
  display.println(connectionType);
  display.display();
}

void updateOLEDStatus(const char* statusMessage) {
  display.fillRect(0, 48, 128, 16, BLACK);
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.print("Status: ");
  display.println(statusMessage);
  display.display();
}

void handleOLEDRefresh() {
  // Later inbox viewer
}
