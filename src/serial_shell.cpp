#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include <FS.h>
#include <SPIFFS.h>

extern std::map<String, uint16_t> reverseMap;
extern bool stickyTopEnabled;

String inputBuffer = "";
char activePrefix = ':'; // Default to shell mode

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
        String content = readFile(filename.c_str());
        if (content.isEmpty()) {
            Serial.println("[Error] File not found or empty.");
        } else {
            Serial.println(content);
        }
    } else if (command.startsWith("rm ")) {
        String filename = command.substring(3);
        if (SPIFFS.remove(filename)) {
            Serial.printf("Deleted: %s\n", filename.c_str());
        } else {
            Serial.printf("[Error] Failed to delete: %s\n", filename.c_str());
        }
    } else if (command == "tokens") {
        Serial.println("Current Token Map:");
        for (const auto& pair : reverseMap) {
            Serial.printf("%u = %s\n", pair.second, pair.first.c_str());
        }
    } else if (command == "top") {
        if (stickyTopEnabled) {
            stickyTopEnabled = false;
            drawPagerScreen("PAGER", "USB");
        } else {
            stickyTopEnabled = true;
            drawTopScreen();
        }
    } else if (command == "help") {
        Serial.println("Shell Commands:");
        Serial.println(": list        - List files");
        Serial.println(": free        - Show RAM and Flash usage");
        Serial.println(": cat <file>  - View file contents");
        Serial.println(": rm <file>   - Delete file");
        Serial.println(": tokens      - List token map");
        Serial.println(": top         - Show OLED system stats");
        Serial.println(": help        - Show this help");
    } else {
        Serial.println("Unknown command.");
    }
}

void handleGreaterThanCommand(const String& line) {
    if (line.startsWith("dm ")) {
        int firstSpace = line.indexOf(' ', 3);
        if (firstSpace == -1) {
            Serial.println("[Error] Usage: > dm <node> <message>");
            return;
        }
        String target = line.substring(3, firstSpace);
        String msg = line.substring(firstSpace + 1);
        String encoded = encodeText(msg);
        sendMessage(encoded, target.c_str());
        Serial.printf("DM to [%s]: %s\n", target.c_str(), msg.c_str());
    } else {
        String encoded = encodeText(line);
        sendMessage(encoded);
        Serial.println("Message sent.");
    }
}

void handleInputLine(const String& line) {
    if (line.length() == 0) return;

    if (line.length() >= 2 && (line[0] == ':' || line[0] == '>' || line[0] == '/' || line[0] == '~') && line[1] == ' ') {
        activePrefix = line[0];
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
            handleGreaterThanCommand(inputBuffer);
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
        } else if (c == 0x08 || c == 127) { // Handle backspace
            if (inputBuffer.length() > 0) {
                inputBuffer.remove(inputBuffer.length() - 1);
                Serial.print("\b \b");
            }
        } else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c);
        }
    }
}
