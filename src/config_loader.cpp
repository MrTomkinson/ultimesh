#include "config_loader.h"
#include <FS.h>
#include <SPIFFS.h>
#include "common_config.h"

std::map<String, String> config;

bool loadConfig() {
    File configFile = SPIFFS.open("/config.ini", FILE_READ);
    if (!configFile || configFile.isDirectory()) {
        Serial.println("[!] Could not open config file.");
        return false;
    }

    while (configFile.available()) {
        String line = configFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line.startsWith("#")) continue;

        int eq = line.indexOf('=');
        if (eq == -1) continue;

        String key = line.substring(0, eq);
        String val = line.substring(eq + 1);
        key.trim(); val.trim();

        runtimeConfig[key] = val;
    }

    configFile.close();
    return true;
}

