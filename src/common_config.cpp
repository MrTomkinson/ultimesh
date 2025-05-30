#include "common_config.h"
#include <Arduino.h>

// Example global values
String startupMode = ":";   // Shell mode
bool autoTop = false;       // Auto OLED on boot
String defaultPath = "/";   // Default working directory
std::map<String, String> runtimeConfig;
void printConfig() {
    Serial.println("[Config Dump]");
    for (auto& pair : runtimeConfig) {
        Serial.printf(" - %s = %s\n", pair.first.c_str(), pair.second.c_str());
    }
}
