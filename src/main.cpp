#include <Arduino.h>
#include "oled_status.h"
#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "config_loader.h"
#include "common_config.h"
#include <FS.h>
#include <SPIFFS.h>

unsigned long lastTopRefresh = 0;
// bool stickyTopEnabled = false;

const char* deviceName = "ULTIMESH";
const char* connectionType = "USB";

void setup() {
  Serial.begin(115200);
  delay(200);


  SPIFFS.begin(true);

  loadConfig("/config.ini");
updateCommonSettings();
  initOLED(deviceName, connectionType);
  initFileSystem();
  loadTokenMap("/tokens/tokens_shell.txt");
  initLoRa();

  // drawPagerScreen(deviceName, connectionType);
  Serial.println("ULTIMESH:$ ");

}

void loop() {
    handleSerialShell();
    handleLoRaTraffic();
    handleOLED();  // <<< NEW

    // Refresh Top Screen periodically
    if (stickyTopEnabled && millis() - lastTopRefresh >= 5000) {
        drawTopScreen();
        lastTopRefresh = millis();
    }

    if (!stickyTopEnabled) {
        // drawPagerScreen(deviceName, connectionType);
    }
}

