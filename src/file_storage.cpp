#include "file_storage.h"
#include <FS.h>
#include <SPIFFS.h>

bool initFileSystem() {
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    return false;
  }
  Serial.println("✅ SPIFFS mounted");
  return true;
}

void listFiles() {
  Serial.println("📁 File List:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.printf(" - %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
}

void showFreeSpace() {
  Serial.printf("🧠 RAM Free: %d KB\n", ESP.getFreeHeap() / 1024);
  Serial.printf("💾 Flash: %d/%d KB\n", SPIFFS.usedBytes() / 1024, SPIFFS.totalBytes() / 1024);
}

bool writeFile(const char* path, const char* content) {
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) return false;
  file.print(content);
  file.close();
  return true;
}

String readFile(const char* path) {
  File file = SPIFFS.open(path, FILE_READ);
  if (!file) return "";
  String content = file.readString();
  file.close();
  return content;
}
