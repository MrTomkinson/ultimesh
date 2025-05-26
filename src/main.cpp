#include <Arduino.h>
#include "oled_status.h"
#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"

unsigned long lastTopRefresh = 0;
bool stickyTopEnabled = false;

const char* deviceName = "ULTIMESH";
const char* connectionType = "USB";

void setup() {
  Serial.begin(115200);
  delay(100);

  initOLED(deviceName, connectionType);
  initFileSystem();
  loadTokenMap("/tokens/tokens_shell.txt");
  initLoRa();

  drawPagerScreen(deviceName, connectionType);
  Serial.println("ULTIMESH:$ ");
}

void loop() {
  handleSerialShell();
  handleLoRaTraffic();

  if (stickyTopEnabled && millis() - lastTopRefresh >= 2000) {
    drawTopScreen();
    lastTopRefresh = millis();
  }

  if (!stickyTopEnabled) {
    drawPagerScreen(deviceName, connectionType);
  }
}
