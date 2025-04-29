// File: src/main.cpp

#include <Arduino.h>
#include "file_storage.h"
#include "oled_status.h"
#include "token_codec.h"
#include "serial_shell.h"
#include "lora_handler.h"
#include <esp_timer.h> // ← We forgot to include this before!

uint64_t bootTime = 0;  // ← Declare it here

void setup() {
  Serial.begin(115200);
  delay(500);

  bootTime = esp_timer_get_time(); // Save boot time!

  initOLED("ULTIMESH", "USB");

  if (initFileSystem()) {
    updateOLEDStatus("SPIFFS OK");
  } else {
    updateOLEDStatus("SPIFFS ERR");
  }

  if (loadTokenMap()) {
    Serial.println("✅ Token map loaded");
  } else {
    Serial.println("❌ Token map failed");
  }

  initLoRa();
}

void loop() {
  handleSerialShell();
  handleLoRaTraffic();
  delay(5); // loop chill
}
