
#pragma once
#include <Arduino.h>
#include <map>

extern std::map<String, String> runtimeConfig;
extern String startupMode;
extern bool autoTop;
extern String defaultPath;

void printConfig();
bool loadConfig();
#define ENABLE_COMMAND_HISTORY 1
#define ENABLE_TAB_COMPLETION  1

#define MAX_HISTORY_ENTRIES    10
#define MAX_TAB_RESULTS        8
