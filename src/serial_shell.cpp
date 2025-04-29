// File: src/serial_shell.cpp

#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include <FS.h>
#include <SPIFFS.h>

String inputBuffer = "";
char activePrefix = ':'; // Default to shell mode

extern std::map<uint16_t, String> tokenMap; // From token_codec

// Update the prompt after every action
void updatePrompt() {
    Serial.printf("ULTIMESH:%c$ ", activePrefix);
}

// Handle ':' prefix (Shell Commands)
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
        File file = SPIFFS.open(filename, FILE_READ);
        if (!file || file.size() == 0) {
            Serial.println("[Error] File not found or empty.");
        } else {
            Serial.println("----- File Start -----");
            while (file.available()) {
                Serial.write(file.read());
            }
            Serial.println("\n----- File End -----");
            file.close();
        }
    } else if (command.startsWith("rm ")) {
        String filename = command.substring(3);
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        if (SPIFFS.remove(filename)) {
            Serial.println("[OK] File deleted.");
        } else {
            Serial.println("[Error] Failed to delete.");
        }
    } else if (command == "tokens") {
        Serial.println("Current Token Map:");
        for (const auto& pair : tokenMap) {
            Serial.printf("%d = %s\n", pair.first, pair.second.c_str());
        }
    } else if (command == "help") {
        Serial.println("Shell Commands:");
        Serial.println(": list        - List files");
        Serial.println(": free        - Show RAM and Flash usage");
        Serial.println(": cat <file>  - View file contents");
        Serial.println(": rm <file>   - Delete file");
        Serial.println(": tokens      - List token map");
        Serial.println(": help        - Show this help");
    } else {
        Serial.println("Unknown command.");
    }
}

// Handle line entered into the serial shell
void handleInputLine(const String& line) {
    if (line.length() == 0) return;

    char prefix = line.charAt(0);

    if (prefix == ':' || prefix == '>' || prefix == '/' || prefix == '~') {
        activePrefix = prefix;
        Serial.printf("[Switched mode to '%c']\n", activePrefix);
        updatePrompt();
        if (line.length() > 2) {
            inputBuffer = line.substring(2);
        } else {
            inputBuffer = "";
        }
    } else {
        inputBuffer = line;
    }

    if (inputBuffer.length() == 0) return; // Don't process if nothing entered

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

    updatePrompt(); // Always reprint prompt
}

// Main shell handler (called inside loop())
void handleSerialShell() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            handleInputLine(inputBuffer);
            inputBuffer = "";
        } else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c); // Live character echo
        }
    }
}
