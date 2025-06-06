#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include "text_editor.h"
#include "common_config.h"
#include "config_loader.h"
#include "ssh_session.h"
#include "UMFrame.h"
#include "command_dispatcher.h"
#include <LoRa.h> // <== Required for `LoRa.beginPacket()`

#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include <vector>
#include <map>
#include <ArduinoJson.h>

// =============================
// Globals
// =============================
extern std::map<String, uint16_t> reverseMap;
extern bool stickyTopEnabled;
extern JsonArray loadedCommandList;

std::vector<String> commandHistory;
int historyIndex = -1;

String inputBuffer = "";
char activePrefix = ':';
char shellReturnMode = ':';
String activeSSHNode = "";
String lastDMTarget = "";
String lastDMFrom = "";

void handleBroadcastCommand(const String& input);
void handleDirectMessageCommand(const String& input);
// =============================
// Prompt / UI
// =============================
void showShellStartupPrompt() {
    Serial.println(F("\n=== Ultimesh Console ==="));
    Serial.println(F("Enter mode:"));
    Serial.println(F("  : → Shell  |  > → LoRa  |  / → Web  |  ~ → BBS  |  # → SSH"));
    Serial.println();
    Serial.print(activePrefix);
    Serial.print(" ");
}

// =============================
// Tab Completion
// =============================
std::vector<String> getMatchingCompletions(const String& prefix, int limit = 10) {
    std::vector<String> matches;

    for (JsonObject cmd : loadedCommandList) {
        String name = cmd["name"] | "";
        if (name.startsWith(prefix)) {
            matches.push_back(name);
        }
        if (cmd.containsKey("aliases")) {
            for (JsonVariant alias : cmd["aliases"].as<JsonArray>()) {
                if (alias.as<String>().startsWith(prefix)) {
                    matches.push_back(alias.as<String>());
                }
            }
        }

        if (matches.size() >= limit) break;
    }

    // Also match filenames
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        String name = String(file.name());
        if (name.startsWith(prefix)) {
            matches.push_back(name);
            if (matches.size() >= limit) break;
        }
        file = root.openNextFile();
    }

    return matches;
}

// =============================
// Input Dispatcher
// =============================
void handleInputLine(const String& line) {
    String trimmed = line;
    trimmed.trim();

    if (trimmed.length() == 1 &&
        (trimmed[0] == ':' || trimmed[0] == '>' || trimmed[0] == '@' || trimmed[0] == '#')) {
        activePrefix = trimmed[0];
        Serial.printf("\n[Switched to %c mode]\n", activePrefix);
        Serial.print(activePrefix); Serial.print(" ");
        return;
    }

    if (trimmed.length() >= 2 &&
        (trimmed[0] == ':' || trimmed[0] == '>' || trimmed[0] == '@' || trimmed[0] == '#') &&
        trimmed[1] == ' ') {
        activePrefix = trimmed[0];
        inputBuffer = trimmed.substring(2);
    } else {
        inputBuffer = trimmed;
    }

switch (activePrefix) {
    case ':': executeCommandByJson(inputBuffer, "shell"); break;
    case '>': handleBroadcastCommand(inputBuffer); break;
    case '@': handleDirectMessageCommand(inputBuffer); break;

    case '#': {
        if (activeSSHNode.length() == 0) {
            String cmd = inputBuffer;
            cmd.trim();
            if (cmd.length() > 0) {
                activeSSHNode = cmd;
                Serial.printf("[SSH] Session opened with %s\n", activeSSHNode.c_str());
            } else {
                Serial.println("[SSH] No session. Use: #<node>");
            }
        } else {
            if (inputBuffer == "exit") {
                Serial.printf("[SSH] Session closed with %s\n", activeSSHNode.c_str());
                activeSSHNode = "";
                activePrefix = '>';
            } else {
                UMFrame frame;
                frame.type = UMFrame::SHELL;
                frame.from = nodeId;
                frame.to = activeSSHNode;
                frame.chunkNumber = 1;
                frame.totalChunks = 1;
                frame.filename = "cmd";
                frame.payload = "[SSH:" + nodeId + "] " + inputBuffer;

                String encoded = frame.encode();
                //Serial.printf("[SSH SEND] to=%s | payload=%s\n", activeSSHNode.c_str(), frame.payload.c_str());
                LoRa.beginPacket();
                LoRa.print(encoded);
                LoRa.endPacket();
            }
        }
        break;
    }


    default: Serial.println("[?] Unknown input prefix."); break;
}


    if (activePrefix == '#' && activeSSHNode.length()) {
        Serial.printf("# [%s] ", activeSSHNode.c_str());
    } else {
        Serial.print(activePrefix);
        Serial.print(" ");
    }

    if (enableCommandHistory && inputBuffer.length() > 0) {
        if (commandHistory.empty() || inputBuffer != commandHistory.back()) {
            if (commandHistory.size() >= maxHistoryEntries)
                commandHistory.erase(commandHistory.begin());
            commandHistory.push_back(inputBuffer);
        }
        historyIndex = commandHistory.size();
    }
}

// =============================
// Serial Shell Input Loop
// =============================
void handleSerialShell() {
    static bool firstRun = true;
    if (firstRun) {
        showShellStartupPrompt();
        firstRun = false;
    }

    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n') {
            handleInputLine(inputBuffer);
            inputBuffer = "";
        }

        else if ((c == 0x08 || c == 127) && inputBuffer.length()) {
            inputBuffer.remove(inputBuffer.length() - 1);
            Serial.print("\b \b");
        }

        else if (enableTabCompletion && c == '\t') {
            int lastSpace = inputBuffer.lastIndexOf(' ');
            String prefix = inputBuffer;
            String preCursor = "";
            if (lastSpace != -1) {
                prefix = inputBuffer.substring(lastSpace + 1);
                preCursor = inputBuffer.substring(0, lastSpace + 1);
            }

            auto matches = getMatchingCompletions(prefix, maxTabResults);
            if (matches.size() == 1) {
                inputBuffer = preCursor + matches[0];
                Serial.print("\r:");
                Serial.print(inputBuffer);
            } else if (matches.size() > 1) {
                Serial.println();
                Serial.println("[Matches]");
                for (String s : matches) Serial.println(" - " + s);
                Serial.print(": ");
                Serial.print(inputBuffer);
            } else {
                Serial.println();
                Serial.println("[No match]");
                Serial.print(": ");
                Serial.print(inputBuffer);
            }
        }

        else if (enableCommandHistory && c == 0x1B) {
            while (!Serial.available());
            if (Serial.read() == '[') {
                char dir = Serial.read();
                if (dir == 'A' && historyIndex > 0) {
                    historyIndex--;
                    inputBuffer = commandHistory[historyIndex];
                    Serial.print("\r:");
                    Serial.print(inputBuffer);
                } else if (dir == 'B') {
                    if (historyIndex < commandHistory.size() - 1) historyIndex++;
                    else historyIndex = commandHistory.size();
                    inputBuffer = (historyIndex < commandHistory.size()) ? commandHistory[historyIndex] : "";
                    Serial.print("\r:");
                    Serial.print(inputBuffer);
                }
            }
        }

        else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c);
        }
    }
}

// =============================
// LoRa and DM Handling
// =============================
void handleBroadcastCommand(const String& input) {
    UMFrame frame;
    frame.type = UMFrame::BROADCAST;
    frame.from = nodeId;
    frame.to = "ALL";
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = "-";
    frame.payload = input;

    String encoded = frame.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

    Serial.printf("[BROADCAST] %s\n", input.c_str());
}

void handleDirectMessageCommand(const String& input) {
    int spaceIdx = input.indexOf(' ');
    if (spaceIdx == -1) {
        Serial.println("[DM] Usage: @<node> <message> or @r <reply>");
        return;
    }

    String rawTarget = input.substring(0, spaceIdx);
    String message = input.substring(spaceIdx + 1);

    if (!rawTarget.startsWith("@")) {
        Serial.println("[DM] Invalid format. Use @<node> <msg>");
        return;
    }

    String target = rawTarget.substring(1);

    if (target == "r") {
        if (lastDMTarget == "") {
            Serial.println("[DM] No recent DM target.");
            return;
        }
        target = lastDMTarget;
    }

    if (target == nodeId) {
        Serial.println("[!] Refusing to DM self.");
        return;
    }

    lastDMTarget = target;

    UMFrame frame;
    frame.type = UMFrame::DIRECT_MSG;
    frame.from = nodeId;
    frame.to = target;
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = "-";
    frame.payload = message;

    String encoded = frame.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

    Serial.printf("[DM] Sent to %s: %s\n", target.c_str(), message.c_str());
}

void handleRemoteSSHCommand(const String& input) {
    if (input == "exit") {
        Serial.printf("[SSH] Session closed with %s\n", activeSSHNode.c_str());
        activeSSHNode = "";
        activePrefix = '>';
        return;
    }

    UMFrame frame;
    frame.type = UMFrame::SHELL;
    frame.from = nodeId;
    frame.to = activeSSHNode;
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = "cmd";
    frame.payload = input;
Serial.printf("[SSH] Preparing command: '%s'\n", input.c_str());
Serial.printf("[SSH] Sending from %s to %s\n", nodeId.c_str(), activeSSHNode.c_str());
Serial.printf("[SSH] Payload: %s\n", input.c_str());
    String encoded = frame.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();
}
