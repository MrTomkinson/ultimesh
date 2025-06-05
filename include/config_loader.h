#pragma once
#include <vector>
#include <Arduino.h>
#include <map>

extern std::map<String, String> runtimeConfig;

void updateCommonSettings();  // in config_loader.h
void printConfig();           // in config_loader.h


bool loadConfig(const String& filename = "/config.ini");
String getConfigValue(const String& key, const String& fallback = "");
int getConfigInt(const String& key, int fallback = 0);
float getConfigFloat(const String& key, float fallback = 0.0);
bool getConfigBool(const String& key, bool fallback = false);
std::vector<String> splitString(const String& s, char delimiter);
