#include "common_config.h"
#include "oled_status.h"
#include "config_loader.h"
#include "lora_handler.h"
#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "json_loader.h"           // ✅ NEW: JSON command loader
#include "command_dispatcher.h"    // ✅ NEW: Dispatcher to execute commands
#include <Arduino.h>
#include <SPIFFS.h>

#define CONFIG_ARDUINO_LOOP_STACK_SIZE 8192

// -- Lora Core 0 Dedicated Task --
void loraTask(void* parameter) {
    while (true) {
        handleLoRaTraffic();  // Non-blocking; processes incoming LoRa
        vTaskDelay(20 / portTICK_PERIOD_MS);  // ~50Hz polling
    }
}

void setup() {
    Serial.begin(115200);
    delay(250);

    Serial.println();
    Serial.println("=== Ultimesh Boot ===");

    // 🔹 Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Mount failed!");
        return;
    } else {
        Serial.println("[OK] SPIFFS Mounted");
    }

    // 🔹 Load configuration
    loadConfig("/config.ini");
    printConfig();

    // 🔹 Load tokens
    loadTokenMap("/tokens_shell.txt");
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());


    // ✅ Load and initialize command system
    if (!loadCommandList("/commands.json")) {
        Serial.println("[Commands] Failed to load command list!");
    }
    initCommandDispatcher();

    // 🔹 Initialize core systems
    initLoRa();
    initOLED(nodeId.c_str(), "LoRa");

    // 🔹 Spawn LoRa receive loop on Core 0
    xTaskCreatePinnedToCore(
        loraTask,             // Function
        "LoRaTask",           // Name
        4096,                 // Stack size
        NULL,                 // Param
        1,                    // Priority
        NULL,                 // Task handle
        0                     // Core 0
    );

    Serial.println("ULTIMESH:$");  // Default CLI prompt
}

void loop() {
    handleOLED();
    handleSerialShell();  // Will internally call executeCommandByJson()
    delay(10);
}
