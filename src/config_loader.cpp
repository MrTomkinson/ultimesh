#include "config_loader.h"
#include <FS.h>
#include <SPIFFS.h>
#include "common_config.h"
#include <vector>
std::map<String, String> runtimeConfig;


bool loadConfig(const String& filename) {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.printf("[config] Could not open %s\n", filename.c_str());
        return false;
    }

    runtimeConfig.clear();
    String currentSection = "";

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.startsWith("#") || line.length() < 3) continue;

        // Handle section headers like [protocol]
        if (line.startsWith("[") && line.endsWith("]")) {
            currentSection = line.substring(1, line.length() - 1);
            continue;
        }

        int eq = line.indexOf('=');
        if (eq == -1) continue;

        String key = line.substring(0, eq);
        String val = line.substring(eq + 1);

        int semicolon = val.indexOf(';');
        if (semicolon != -1) val = val.substring(0, semicolon);

        key.trim();
        val.trim();

        if (key.length()) {
            String fullKey = currentSection.length() ? currentSection + "." + key : key;
            runtimeConfig[fullKey] = val;
        }
    }

    file.close();
    Serial.printf("[config] Loaded %d settings from %s\n", runtimeConfig.size(), filename.c_str());
    updateCommonSettings();  // <-- sync into C++ variables
    return true;
}

std::vector<String> splitString(const String& s, char delimiter) {
    std::vector<String> tokens;
    String token = "";
    for (char c : s) {
        if (c == delimiter) {
            if (token.length()) tokens.push_back(token);
            token = "";
        } else {
            token += c;
        }
    }
    if (token.length()) tokens.push_back(token);
    return tokens;
}

String getConfigValue(const String& key, const String& fallback) {
    if (runtimeConfig.count(key)) return runtimeConfig[key];
    return fallback;
}

int getConfigInt(const String& key, int fallback) {
    if (runtimeConfig.count(key)) return runtimeConfig[key].toInt();
    return fallback;
}

float getConfigFloat(const String& key, float fallback) {
    if (runtimeConfig.count(key)) return runtimeConfig[key].toFloat();
    return fallback;
}

bool getConfigBool(const String& key, bool fallback) {
    if (runtimeConfig.count(key)) {
        String val = runtimeConfig[key];
        val.toLowerCase();
        return val == "1" || val == "true" || val == "yes" || val == "on";
    }
    return fallback;
}

void printConfig(Print* out){
    out->println("== Runtime Config ==");
    for (const auto& pair : runtimeConfig) {
        out->printf("  %s = %s\n", pair.first.c_str(), pair.second.c_str());
    }
}
