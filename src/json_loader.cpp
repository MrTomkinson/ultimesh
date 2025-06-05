#include "json_loader.h"
#include <SPIFFS.h>

// Allocate a persistent JSON document (~16 KB)
DynamicJsonDocument commandJsonDoc(16384);
JsonArray loadedCommandList;

bool loadCommandList(const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.printf("[json] ❌ Cannot open %s\n", path);
        return false;
    }

    DeserializationError err = deserializeJson(commandJsonDoc, file);
    file.close();

    if (err) {
        Serial.printf("[json] ❌ Parse failed: %s\n", err.c_str());
        return false;
    }

    if (!commandJsonDoc.containsKey("commands")) {
        Serial.println("[json] ❌ Missing 'commands' key in JSON");
        return false;
    }

    loadedCommandList = commandJsonDoc["commands"].as<JsonArray>();
    Serial.printf("[json] ✅ Loaded %d commands from %s\n", loadedCommandList.size(), path);
    return true;
}
