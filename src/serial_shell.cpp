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
#include "chunked_transfer.h"
#include "common_utils.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include <vector>
#include <map>
#include <ArduinoJson.h>

// Globals
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

// Prompt Display
void showShellStartupPrompt() {
    Serial.println(F("\n=== Ultimesh Console ==="));
    Serial.println(F("Enter mode:"));
    Serial.println(F("  : → Shell  |  > → LoRa  |  @ → DM  |  # → SSH"));
    Serial.println();
    Serial.print(activePrefix);
    Serial.print(" ");
}

// Handle LoRa broadcast command
void handleBroadcastCommand(const String& input) {
    UMFrame frame;
    frame.type = UMFrame::BROADCAST;
    frame.from = toBytes(nodeId);
    frame.to = toBytes("ALL");
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = toBytes("-");
    frame.payload = toBytes(input);
    sendLargePayload("ALL", "-", input, frame.type);
    Serial.printf("[BROADCAST] %s\n", input.c_str());
}

// Handle DM
void handleDirectMessageCommand(const String& input) {
    int spaceIdx = input.indexOf(' ');
    if (spaceIdx == -1) {
        Serial.println("[DM] Usage: @<node> <msg>");
        return;
    }

    String rawTarget = input.substring(0, spaceIdx);
    String message = input.substring(spaceIdx + 1);

    if (!rawTarget.startsWith("@")) {
        Serial.println("[DM] Format: @<node> <msg>");
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
        Serial.println("[!] Cannot DM self.");
        return;
    }

    lastDMTarget = target;

    UMFrame frame;
    frame.type = UMFrame::DIRECT_MSG;
    frame.from = toBytes(nodeId);
    frame.to = toBytes(target);
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = toBytes("-");
    frame.payload = toBytes(message);
    sendLargePayload(target, "-", message, frame.type);
    Serial.printf("[DM] Sent to %s: %s\n", target.c_str(), message.c_str());
}

// SSH Command Handler
void handleRemoteSSHCommand(const String& input) {
    if (input == "exit") {
        Serial.printf("[SSH] Session closed with %s\n", activeSSHNode.c_str());
        activeSSHNode = "";
        activePrefix = '>';
        return;
    }

    UMFrame frame;
    frame.type = UMFrame::SHELL;
    frame.from = toBytes(nodeId);
    frame.to = toBytes(activeSSHNode);
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = toBytes("cmd");
    frame.payload = toBytes(input);
    sendLargePayload(activeSSHNode, "cmd", fromBytes(frame.payload), UMFrame::SSH);

}

// Get completions
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

// Main serial input handler
void handleInputLine(const String& line) {
    String trimmed = line;
    trimmed.trim();

    if (trimmed.length() == 1 && (trimmed[0] == ':' || trimmed[0] == '>' || trimmed[0] == '@' || trimmed[0] == '#')) {
        activePrefix = trimmed[0];
        Serial.printf("\n[Switched to %c mode]\n", activePrefix);
        Serial.print(activePrefix); Serial.print(" ");
        return;
    }

    if (trimmed.length() >= 2 && trimmed[1] == ' ') {
        activePrefix = trimmed[0];
        inputBuffer = trimmed.substring(2);
    } else {
        inputBuffer = trimmed;
    }

    switch (activePrefix) {
        case ':': executeCommandByJson(inputBuffer, "shell"); break;
        case '>': handleBroadcastCommand(inputBuffer); break;
        case '@': handleDirectMessageCommand(inputBuffer); break;
        case '#':
            if (activeSSHNode == "") {
                activeSSHNode = inputBuffer;
                Serial.printf("[SSH] Connected to %s\n", activeSSHNode.c_str());
            } else {
                handleRemoteSSHCommand(inputBuffer);
            }
            break;
        default: Serial.println("[?] Unknown prefix."); break;
    }

    if (enableCommandHistory && inputBuffer.length() > 0) {
        if (commandHistory.empty() || inputBuffer != commandHistory.back()) {
            if (commandHistory.size() >= maxHistoryEntries)
                commandHistory.erase(commandHistory.begin());
            commandHistory.push_back(inputBuffer);
        }
        historyIndex = commandHistory.size();
    }

    Serial.print((activePrefix == '#' && activeSSHNode != "") ? "# [" + activeSSHNode + "] " : String(activePrefix) + " ");
}

// Loop reader
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
        } else if ((c == 0x08 || c == 127) && inputBuffer.length()) {
            inputBuffer.remove(inputBuffer.length() - 1);
            Serial.print("\b \b");
        } else if (enableTabCompletion && c == '\t') {
            int lastSpace = inputBuffer.lastIndexOf(' ');
            String prefix = lastSpace == -1 ? inputBuffer : inputBuffer.substring(lastSpace + 1);
            String preCursor = lastSpace == -1 ? "" : inputBuffer.substring(0, lastSpace + 1);

            auto matches = getMatchingCompletions(prefix, maxTabResults);
            if (matches.size() == 1) {
                inputBuffer = preCursor + matches[0];
                Serial.print("\r:");
                Serial.print(inputBuffer);
            } else {
                Serial.println();
                for (auto& m : matches) Serial.println(" - " + m);
                Serial.print(": ");
                Serial.print(inputBuffer);
            }
        } else if (enableCommandHistory && c == 0x1B) {
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
                    inputBuffer = historyIndex < commandHistory.size() ? commandHistory[historyIndex] : "";
                    Serial.print("\r:");
                    Serial.print(inputBuffer);
                }
            }
        } else if (c != '\r') {
            inputBuffer += c;
            Serial.print(c);
        }
    }
}
