#pragma once
#include <ArduinoJson.h>

// Global JSON document to keep the command list in memory
extern DynamicJsonDocument commandJsonDoc;

// This is a view into `commandJsonDoc["commands"]`
extern JsonArray loadedCommandList;

// Loads the command list from SPIFFS
bool loadCommandList(const char* path = "/commands.json");
