#pragma once

#include <Arduino.h>
#include <map>

extern std::map<String, String> runtimeConfig;

bool loadConfig(const String& filename = "/config.ini");
void printConfig();

String getConfigValue(const String& key, const String& fallback = "");
int    getConfigInt(const String& key, int fallback = 0);
float  getConfigFloat(const String& key, float fallback = 0.0f);
bool   getConfigBool(const String& key, bool fallback = false);
