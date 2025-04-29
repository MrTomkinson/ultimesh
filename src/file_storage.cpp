#include "file_storage.h"
#include <FS.h>
#include <SPIFFS.h>

bool initFileSystem() {
    if (!SPIFFS.begin(true)) {
        Serial.println("[Error] SPIFFS Mount Failed!");
        return false;
    }
    Serial.println("[OK] SPIFFS Mounted");
    return true;
}

void listFiles() {
    Serial.println("File List:");
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory()) {
        Serial.println("[Error] Failed to open root directory.");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        Serial.printf("- %s (%d bytes)\n", file.path(), file.size());
        file = root.openNextFile();
    }
}

String readFile(const char* path) {
    String fixedPath = String(path);
    if (!fixedPath.startsWith("/")) {
        fixedPath = "/" + fixedPath;
    }

    File file = SPIFFS.open(fixedPath, "r");
    if (!file) {
        Serial.printf("[Error] Cannot open file: %s\n", fixedPath.c_str());
        return "";
    }

    if (file.size() == 0) {
        Serial.printf("[Error] Empty file: %s\n", fixedPath.c_str());
        file.close();
        return "";
    }

    String content;
    while (file.available()) {
        content += (char)file.read();
    }

    file.close();
    return content;
}
