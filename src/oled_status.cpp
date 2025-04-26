// oled_status.cpp
// OLED Status Display Functions for ULTIMESH

#include "oled_status.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include <SPIFFS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static char currentDeviceName[32];
static char currentConnectionType[16];

// Initialize the OLED Display
void initOLED(const char* deviceName, const char* connectionType) {
    // Wake up OLED power pin if necessary
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
    delay(100);

    // Wire.begin(sda, scl) - Heltec V2 uses GPIO 4 + 15
    Wire.begin(4, 15);

    strncpy(currentDeviceName, deviceName, sizeof(currentDeviceName) - 1);
    strncpy(currentConnectionType, connectionType, sizeof(currentConnectionType) - 1);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[OLED] SSD1306 init failed at 0x3C"));
        return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("🏄 ULTIMESH 🏄");
    display.println("v0.A1 - Ride the wave!");

    display.display();
    delay(1000); // Pause on splash
}

// Quick text update to bottom of OLED
void updateOLEDStatus(const char* statusMessage) {
    display.fillRect(0, 48, 128, 16, BLACK); // Clear bottom section
    display.setCursor(0, 48);
    display.setTextSize(1);
    display.print("Status: ");
    display.println(statusMessage);
    display.display();
}

// Regular stats update (ID, RAM, Flash, LoRa)
void updateOLEDStats() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    // Show static name
    display.println(currentDeviceName);
    display.print("Conn: ");
    display.println(currentConnectionType);

    // Node ID
    uint64_t chipid = ESP.getEfuseMac();
    display.printf("ID: %04X\n", (uint32_t)(chipid >> 32));

    // RAM
    uint32_t freeHeap = ESP.getFreeHeap();
    display.printf("RAM: %dKB\n", freeHeap / 1024);

    // SPIFFS Storage
    if (SPIFFS.begin(false)) {
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        display.printf("FS: %d/%dKB", used / 1024, total / 1024);
    } else {
        display.println("FS: MOUNT ERR");
    }

    // LoRa Status [Future improvement: actual status checking]
    display.setCursor(0, 56);
    display.setTextSize(1);
    display.print("LoRa: OK");

    display.display();
}
