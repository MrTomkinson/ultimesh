#include "token_codec.h"
#include <FS.h>
#include <SPIFFS.h>

std::map<uint16_t, String> tokenMap;
std::map<String, uint16_t> reverseMap;

bool loadTokenMap(const char* path) {
    tokenMap.clear();
    reverseMap.clear();

    File file = SPIFFS.open(path);
    if (!file) {
        Serial.print("❌ Failed to open token map at ");
        Serial.println(path);
        return false;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        int eq = line.indexOf('=');
        if (eq > 0) {
            uint16_t code = line.substring(0, eq).toInt();
            String text = line.substring(eq + 1);
            tokenMap[code] = text;
            reverseMap[text] = code;
        }
    }

    file.close();
    Serial.print("✅ Token map loaded: ");
    Serial.println(path);
    return true;
}

String encodeText(const String& raw) {
    String result = raw;
    for (const auto& pair : reverseMap) {
        result.replace(pair.first, String("▒") + String(pair.second));  // '▒' prefix
    }
    return result;
}

String decodeTokens(const String& compressed) {
    String result = compressed;
    int idx = 0;

    while ((idx = result.indexOf("▒", idx)) != -1) {
        int start = idx + 1;
        int end = start;

        while (end < result.length() && isDigit(result[end])) {
            end++;
        }

        uint16_t code = result.substring(start, end).toInt();
        String replacement = tokenMap.count(code) ? tokenMap[code] : String("▒") + String(code);
        result.replace("▒" + String(code), replacement);

        idx = end;
    }

    return result;
}
