#include <Arduino.h>
#include "oled_status.h"
#include "file_storage.h"
#include "token_codec.h"
#include "serial_shell.h"
#include "lora_handler.h"
#include <FS.h>
#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("🏄 ULTIMESH 🏄");
  Serial.println("v0.A1 - \"Ride the wave!\"");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed!");
  } else {
    Serial.println("✅ SPIFFS mounted");
  }

  // Load Token Map
  if (loadTokenMap("/tokens_shell.txt")) {
    Serial.println("✅ Token map loaded");
  } else {
    Serial.println("⚠️ No token map found (continuing)");
  }

  // Init OLED
  initOLED("ULTIMESH", "USB");

  // Init LoRa
  Serial.println("🔌 Initializing LoRa...");
  initLoRa();
}

void loop() {
  handleSerialShell();
  handleLoRaTraffic();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) { // Refresh OLED stats every 5 sec
    updateOLEDStats();
    lastUpdate = millis();
  }
}
