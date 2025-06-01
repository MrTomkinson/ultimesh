#include "oled_status.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <esp_system.h>
#include "common_config.h" // ⬅ for oledDisplayDuration and stickyTopEnabled

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String lastSender = "LoRa";
String lastMessage = "Waiting...";
unsigned long lastLoRaMessageTime = 0;
bool isLatched = false;

OledMode currentMode = MODE_TOP;
OledMode returnMode = MODE_TOP;
static OledMode lastDisplayedMode = MODE_MESSAGE;

void initOLED(const char* deviceName, const char* connectionType) {
    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
    delay(100);

    Wire.begin(4, 15);  // SDA = 4, SCL = 15
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[OLED] SSD1306 init failed");
        return;
    }

    currentMode = stickyTopEnabled ? MODE_TOP : MODE_MESSAGE;
    returnMode = currentMode;
    lastDisplayedMode = MODE_MESSAGE; // force first draw

    drawTopScreen(); // Initial screen
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

void drawMessageScreen(const String& sender, const String& msg) {
    display.setRotation(0);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.printf("[%s]", sender.c_str());

    int y = 12;
    int lineLen = 21;

    for (int i = 0; i < msg.length(); i += lineLen) {
        display.setCursor(0, y);
        display.println(msg.substring(i, i + lineLen));
        y += 10;
        if (y > 54) break;
    }

    display.display();
}

void showLoRaMessage(const String& sender, const String& msg, int durationMs) {
    lastSender = sender;
    lastMessage = msg;

    lastLoRaMessageTime = millis();
    isLatched = true;
    currentMode = MODE_MESSAGE;

    drawMessageScreen(sender, msg);
}

void handleOLED() {
    unsigned long now = millis();

    // Handle timeout
    if (isLatched && (now - lastLoRaMessageTime >= oledDisplayDuration)) {
        isLatched = false;
        currentMode = returnMode;
    }

    // Only redraw if mode changed
    if (currentMode == lastDisplayedMode) return;

    switch (currentMode) {
        case MODE_TOP:
            drawTopScreen();
            break;
        case MODE_MESSAGE:
            drawMessageScreen(lastSender, lastMessage);
            break;
    }

    lastDisplayedMode = currentMode;
}
