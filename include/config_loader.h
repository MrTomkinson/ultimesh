#pragma once

#include <Arduino.h>
#include <map>

extern std::map<String, String> config;

bool loadConfig();
String getConfig(const String& key, const String& defaultValue = "");
void setConfig(const String& key, const String& value);
void printConfig();
