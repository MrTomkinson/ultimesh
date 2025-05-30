#include <Arduino.h>
#include "oled_status.h"
#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "config_loader.h"
#include <FS.h>
#include <SPIFFS.h>

unsigned long lastTopRefresh = 0;
bool stickyTopEnabled = false;

const char* deviceName = "ULTIMESH";
const char* connectionType = "USB";

void setup() {
  Serial.begin(115200);
  delay(100);
  SPIFFS.begin(true);
  loadConfig();  // Loads /config.ini from SPIFFS



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

  if (!loadConfig()) {
  Serial.println("[!] Config load failed.");
}


}
