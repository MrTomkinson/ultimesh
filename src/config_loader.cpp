#include "config_loader.h"
#include <FS.h>
#include <SPIFFS.h>
#include "common_config.h"
std::map<String, String> runtimeConfig;

bool loadConfig(const String& filename) {
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.printf("[config] Could not open %s\n", filename.c_str());
        return false;
    }

    runtimeConfig.clear();
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("#") || line.length() < 3) continue;

        int eq = line.indexOf('=');
        if (eq == -1) continue;

        String key = line.substring(0, eq);
        String val = line.substring(eq + 1);

        int semicolon = val.indexOf(';');  // optional inline comments
        if (semicolon != -1) val = val.substring(0, semicolon);

        key.trim();
        val.trim();

        if (key.length()) runtimeConfig[key] = val;
    }

    file.close();
    Serial.printf("[config] Loaded %d settings from %s\n", runtimeConfig.size(), filename.c_str());
      updateCommonSettings();  // <-- sync .cpp config values from file
    return true;
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
