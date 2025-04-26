#include <Arduino.h>
#include "lora_handler.h"
#include "parallel_input.h"
#include "serial_shell.h"
#include "oled_status.h"
#include "file_storage.h"
#include "token_codec.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("✅ ULTIMESH Booting...");

  if (initFileSystem()) {
    loadTokenMap("/tokens.txt");
  }

  initOLED("ULTIMESH PAGER", "USB");
  initLoRa();
  initParallelInput();
  initSerialShell();

  updateOLEDStatus("System Ready");
}

void loop() {
  handleLoRaTraffic();
  handleParallelInput();
  handleSerialShell();
  handleOLEDRefresh();
  delay(10);
}
