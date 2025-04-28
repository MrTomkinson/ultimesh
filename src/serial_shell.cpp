// File: serial_shell.cpp

#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include <FS.h>
#include <SPIFFS.h>

String inputBuffer = "";
char activePrefix = ':'; // Default to shell mode

void handleColonCommand(const String& command) {
    if (command == "list") {
        listFiles();
    } else if (command == "free") {
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        Serial.printf("🧠 RAM Free: %d KB\n", ESP.getFreeHeap() / 1024);
        Serial.printf("💾 Flash: %d/%d KB\n", used / 1024, total / 1024);
    } else if (command.startsWith("cat ")) {
        String filename = command.substring(4);
        String content = readFile(filename.c_str());
        Serial.println(content);
    } else if (command.startsWith("rm ")) {
        String filename = command.substring(3);
        if (SPIFFS.remove(filename)) {
            Serial.printf("✅ Deleted %s\n", filename.c_str());
        } else {
            Serial.printf("❌ Failed to delete %s\n", filename.c_str());
        }
    } else if (command == "help") {
        Serial.println("📖 Shell Commands:");
        Serial.println(": list      - List files");
        Serial.println(": free      - Show RAM and Flash usage");
        Serial.println(": cat <file> - View file contents");
        Serial.println(": rm <file> - Delete file");
        Serial.println(": help      - Show this help");
    } else {
        Serial.println("❓ Unknown command");
    }
}

void handleInputLine(const String& line) {
    if (line.length() == 0) return;

    // Only allow switch if prefix + space detected
    if ((line.charAt(0) == ':' || line.charAt(0) == '>' || line.charAt(0) == '/' || line.charAt(0) == '~')
        && line.charAt(1) == ' ') {
        activePrefix = line.charAt(0);
        Serial.printf("🔁 Switched mode to '%c'\n", activePrefix);
        inputBuffer = line.substring(2); // skip prefix and space
    } else {
        inputBuffer = line; // No mode switch, use current active mode
    }

    switch (activePrefix) {
        case ':':
            handleColonCommand(inputBuffer);
            break;
        case '>':
            Serial.printf("📤 Sending message: %s\n", encodeText(inputBuffer).c_str());
            sendMessage(encodeText(inputBuffer));
            break;
        case '/':
            Serial.println("🌐 Web nav not ready yet");
            break;
        case '~':
            Serial.println("📟 BBS mode coming soon");
            break;
    }
}

void handleSerialShell() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            handleInputLine(inputBuffer);
            inputBuffer = "";
        } else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c); // Live echo
        }
    }
}
