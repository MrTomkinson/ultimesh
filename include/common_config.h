#pragma once

#include <Arduino.h>
#include <map>

// Global config map populated from /config.ini
extern std::map<String, String> runtimeConfig;

// === Feature Toggles ===
extern bool enableCommandHistory;
extern bool enableTabCompletion;
extern int  maxHistoryEntries;
extern int  maxTabResults;

// === System Identity ===
extern String nodeId;
extern String userName;
extern String defaultMode;
extern String shellMode;
extern String startupCommand;
extern String defaultTokenMap;

// === LoRa Radio Settings ===
extern int   sendDelayMs;
extern int   maxPacketSize;
extern int   spreadFactor;
extern int   txPower;
extern float frequency;
extern bool  locationEnabled;

// === OLED / UI ===
extern int  oledDisplayDuration;
extern bool showHelpOnStart;
extern bool flashOnLoRa;
extern bool showTop;
extern bool stickyTopEnabled;

// === Sync updated values from runtimeConfig
void updateCommonSettings();

// === Print loaded values to serial
void printConfig();
