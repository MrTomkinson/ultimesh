// ULTIMESH Firmware - main.cpp
// v0.A1 - "Ride the wave!"
// Baseline with OLED live updates (5-second refresh)

#include <Arduino.h>
#include <Wire.h>
#include "oled_status.h"
#include "lora_handler.h"
#include "token_codec.h"
#include <SPIFFS.h>

// === Settings ===
#define OLED_REFRESH_INTERVAL 5000  // 5 seconds

// === Global Variables ===
unsigned long lastOLEDUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n🏄 ULTIMESH 🏄 v0.A1 - \"Ride the wave!\""));
  Serial.println(F("✅ Booting..."));

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println(F("❌ SPIFFS Mount Failed"));
  } else {
    Serial.println(F("✅ SPIFFS mounted"));
  }

  // Load Token Table
  if (loadTokenMap("/tokens.txt")) {
    Serial.println(F("✅ Token map loaded"));
  } else {
    Serial.println(F("⚠️ No token table found"));
  }

  // Initialize OLED
  initOLED("ULTIMESH", "USB");

  // Initialize LoRa
  Serial.println(F("🔌 Initializing LoRa..."));
  initLoRa();
}

void loop() {
  // === OLED Live Update ===
  if (millis() - lastOLEDUpdate >= OLED_REFRESH_INTERVAL) {
    updateOLEDStats();
    lastOLEDUpdate = millis();
  }

  // === LoRa Traffic Handler ===
  handleLoRaTraffic();

  // === (Placeholder) Serial Shell Input Handler ===
  // handleSerialShellInput();  // [TODO: Add command shell]

  delay(10); // Small yield
}
