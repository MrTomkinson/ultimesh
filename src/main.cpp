#include "common_config.h"
#include "oled_status.h"
#include "config_loader.h"
#include "lora_handler.h"
#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
// Removed initTextEditor.h since it doesn't declare initTextEditor()

#include <Arduino.h>
#include <SPIFFS.h>

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

    if (!SPIFFS.begin(true)) {
        Serial.println("[SPIFFS] Mount failed!");
        return;
    } else {
        Serial.println("[OK] SPIFFS Mounted");
    }

    loadConfig("/config.ini");
    printConfig();

    loadTokenMap("/tokens_shell.txt");

    initLoRa();
    initOLED(nodeId.c_str(), "LoRa");

    // Spawn LoRa receive loop on Core 0
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
    handleSerialShell();
    delay(10);
}
