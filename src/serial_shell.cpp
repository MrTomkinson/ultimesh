// File: src/serial_shell.cpp

#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include <FS.h>
#include <SPIFFS.h>

extern std::map<uint16_t, String> tokenMap;
extern bool stickyTopEnabled;
extern void drawTopScreen();

String inputBuffer = "";
char activePrefix = ':'; // Default shell mode

void handleColonCommand(const String& command) {
    if (command == "list") {
        listFiles();
    } else if (command == "free") {
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        Serial.printf("RAM Free: %d KB\n", ESP.getFreeHeap() / 1024);
        Serial.printf("Flash: %d/%d KB\n", used / 1024, total / 1024);
    } else if (command.startsWith("cat ")) {
        String filename = command.substring(4);
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        String content = readFile(filename.c_str());
        if (content.length() > 0) {
            Serial.println(content);
        } else {
            Serial.println("[Error] Empty file or file not found.");
        }
    } else if (command.startsWith("rm ")) {
        String filename = command.substring(3);
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        if (SPIFFS.exists(filename)) {
            SPIFFS.remove(filename);
            Serial.printf("[Info] Deleted: %s\n", filename.c_str());
        } else {
            Serial.println("[Error] File not found.");
        }
    } else if (command == "tokens") {
        Serial.println("Current Token Map:");
        for (const auto& pair : tokenMap) {
            Serial.printf("%d = %s\n", pair.first, pair.second.c_str());
        }
    } else if (command == "top") {
        stickyTopEnabled = !stickyTopEnabled;
        Serial.printf("[System] Top Sticky mode: %s\n", stickyTopEnabled ? "ON" : "OFF");
    } else if (command == "topx") {
        stickyTopEnabled = false;
        Serial.println("[System] Top Sticky forcibly disabled.");
    } else if (command == "help") {
        Serial.println("Shell Commands:");
        Serial.println(": list        - List files");
        Serial.println(": free        - Show RAM and Flash usage");
        Serial.println(": cat <file>  - View file contents");
        Serial.println(": rm <file>   - Delete file");
        Serial.println(": tokens      - List token map");
        Serial.println(": top         - Toggle system monitor");
        Serial.println(": topx        - Force disable monitor");
        Serial.println(": help        - Show this help");
    } else {
        Serial.println("Unknown command.");
    }
}

void handleInputLine(const String& line) {
    if (line.length() == 0) return;

    char prefix = line.charAt(0);

    if (prefix == ':' || prefix == '>' || prefix == '/' || prefix == '~') {
        activePrefix = prefix;
        Serial.printf("[Switched mode to '%c']\n", activePrefix);
        inputBuffer = line.substring(2);
    } else {
        inputBuffer = line;
    }

    switch (activePrefix) {
        case ':':
            handleColonCommand(inputBuffer);
            break;
        case '>':
            Serial.printf("Sending message: %s\n", encodeText(inputBuffer).c_str());
            sendMessage(encodeText(inputBuffer));
            break;
        case '/':
            Serial.println("[Web nav not ready yet]");
            break;
        case '~':
            Serial.println("[BBS mode coming soon]");
            break;
    }
}

void handleSerialShell() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            handleInputLine(inputBuffer);
            inputBuffer = "";
            Serial.print("ULTIMESH:$ ");
        } else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c); // Live echo
        }
    }
}
