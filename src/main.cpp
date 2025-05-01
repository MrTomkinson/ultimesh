#include <Arduino.h>
#include "oled_status.h"
#include "serial_shell.h"
#include "token_codec.h"
#include "file_storage.h"
#include "lora_handler.h"
#include <SPIFFS.h>

unsigned long bootTime = 0;
bool stickyTopEnabled = false;
unsigned long lastTopUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  initOLED("ULTIMESH", "USB");
  drawPagerScreen("ULTIMESH", "USB");

  if (!initFileSystem()) {
    Serial.println("[ERROR] Failed to init SPIFFS");
    return;
  }

  loadTokenMap("/tokens/tokens_shell.txt");
  initLoRa();

  bootTime = millis();
}

void loop() {
  handleSerialShell();

  if (stickyTopEnabled) {
    if (millis() - lastTopUpdate > 1000) {
      drawTopScreen();
      lastTopUpdate = millis();
    }
  } else {
    drawPagerScreen("ULTIMESH", "USB");
  }

  delay(10); // Short delay to reduce input lag
}
