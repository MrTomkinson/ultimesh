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
#include <LoRa.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include <vector>
#include <map>


void handleBroadcastCommand(const String& input);
void handleDirectMessageCommand(const String& input);
void handleSSHCommand(const String& input);



// =============================
// Globals
// =============================
std::vector<String> getMatchingCompletions(const String& prefix, int limit = 10);

extern std::map<String, uint16_t> reverseMap;
extern bool stickyTopEnabled;

void sendLoRaMessage(const String& message, const String& target);

String inputBuffer = "";
char activePrefix = ':';           // Current shell mode
char shellReturnMode = ':';        // Used to return after command
String activeSSHNode = "";         // Target for # SSH mode

std::vector<String> commandHistory;
int historyIndex = -1;
String lastDMFrom = "";

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
// Tokenizer
// =============================
std::vector<String> tokenize(const String &input) {
    std::vector<String> tokens;
    String token = "";
    for (unsigned int i = 0; i < input.length(); i++) {
        if (input[i] == ' ') {
            if (token.length()) {
                tokens.push_back(token);
                token = "";
            }
        } else {
            token += input[i];
        }
    }
    if (token.length()) tokens.push_back(token);
    return tokens;
}

// =============================
// Tab Completion
// =============================
std::vector<String> getAllCommands() {
    return {
        "ls", "list", "dir", "cat", "rm", "edit", "cp", "mv", "touch", "echo",
        "tokens", "top", "clear", "cls", "help", "config"
    };
}

std::vector<String> getMatchingCompletions(const String& prefix, int limit) {
    std::vector<String> matches;

    for (String cmd : getAllCommands()) {
        if (cmd.startsWith(prefix)) {
            matches.push_back(cmd);
            if (matches.size() >= limit) break;
        }
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

// =============================
// LoRa > Command Handler
// =============================
void handleGreaterThanCommand(const String& inputLine) {
    String input = inputLine;
    input.trim();

    if (input.startsWith("/dm ")) {
        int space = input.indexOf(' ', 4);
        if (space > 0) {
            String target = input.substring(4, space);
            String msg = input.substring(space + 1);

            if (msg == "ssh") {
                activeSSHNode = target;
                activePrefix = '#';
                Serial.printf("[SSH] Session opened with %s\n", target.c_str());
                return;
            }

            UMFrame frame;
            frame.type = UMFrame::DIRECT_MSG;
            frame.from = nodeId;
            frame.to = target;
            frame.chunkNumber = 1;
            frame.totalChunks = 1;
            frame.filename = "-";
            frame.payload = msg;

            String encoded = frame.encode();
            LoRa.beginPacket();
            LoRa.print(encoded);
            LoRa.endPacket();

            Serial.printf("[SHELL] Sent to %s: %s\n", target.c_str(), msg.c_str());
            return;
        }
    }

    // Fallback: plain broadcast
    UMFrame bcast;
    bcast.type = UMFrame::BROADCAST;
    bcast.from = nodeId;
    bcast.to = "BCAST";
    bcast.chunkNumber = 1;
    bcast.totalChunks = 1;
    bcast.filename = "-";
    bcast.payload = input;

    String encoded = bcast.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

    Serial.printf("[SHELL] Broadcast: %s\n", input.c_str());
}

// =============================
// Serial Shell : Mode
// =============================
void handleColonCommand(const String& rawCommand) {
    String command = rawCommand;
    command.trim();
    command.replace("\\", "/");

    if (command.startsWith("echo ")) {
        int split = command.indexOf('>');
        if (split > -1) {
            String text = command.substring(5, split);
            text.trim();
            String path = command.substring(split + 1);
            path.trim();
            if (!path.startsWith("/")) path = "/" + path;
            File f = SPIFFS.open(path, FILE_WRITE);
            if (!f) Serial.printf("[!] Failed to open %s\n", path.c_str());
            else {
                f.println(text);
                f.close();
                Serial.printf("[✓] Wrote to %s\n", path.c_str());
            }
            Serial.println();
            return;
        }
    }

    std::vector<String> tokens = tokenize(command);
    if (tokens.empty()) return;
    String cmd = tokens[0];

    if (cmd == "ls" || cmd == "list" || cmd == "dir") {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file) {
            Serial.printf("       %-24s %6d bytes\n", file.name(), file.size());
            file = root.openNextFile();
        }
    }

    else if (cmd == "config") printConfig();

    else if (cmd == "cat" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        String content = readFile(file.c_str());
        if (content.isEmpty()) Serial.println("[!] Not found or empty.");
        else Serial.println(content);
    }

    else if (cmd == "rm" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        if (SPIFFS.remove(file)) Serial.printf("[✓] Deleted: %s\n", file.c_str());
        else Serial.printf("[!] Failed to delete: %s\n", file.c_str());
    }

    else if (cmd == "edit" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        launchTextEditor(file);
        shellReturnMode = activePrefix;
        activePrefix = ':'; // force shell
    }

    else if (cmd == "cp" && tokens.size() > 2) {
        String src = tokens[1], dst = tokens[2];
        if (!src.startsWith("/")) src = "/" + src;
        if (!dst.startsWith("/")) dst = "/" + dst;
        File in = SPIFFS.open(src, FILE_READ);
        File out = SPIFFS.open(dst, FILE_WRITE);
        if (!in || !out) Serial.println("[!] Copy failed.");
        else {
            while (in.available()) out.write(in.read());
            Serial.printf("[✓] Copied %s → %s\n", src.c_str(), dst.c_str());
        }
        in.close(); out.close();
    }

    else if (cmd == "mv" && tokens.size() > 2) {
        String src = tokens[1], dst = tokens[2];
        if (!src.startsWith("/")) src = "/" + src;
        if (!dst.startsWith("/")) dst = "/" + dst;
        if (SPIFFS.rename(src, dst)) Serial.printf("[✓] Renamed to: %s\n", dst.c_str());
        else Serial.println("[!] Rename failed.");
    }

    else if (cmd == "touch" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        File f = SPIFFS.open(file, FILE_WRITE);
        if (f) {
            f.close();
            Serial.printf("[✓] Created: %s\n", file.c_str());
        } else Serial.println("[!] Failed to create.");
    }

    else if (cmd == "top") {
        if (stickyTopEnabled) {
            currentMode = MODE_TOP;
            returnMode = MODE_TOP;
        } else {
            currentMode = MODE_MESSAGE;
            returnMode = MODE_MESSAGE;
        }
    }

    else if (cmd == "tokens") {
        for (auto &pair : reverseMap) {
            Serial.printf("%u = %s\n", pair.second, pair.first.c_str());
        }
    }

    else if (cmd == "clear" || cmd == "cls") Serial.print("\033[2J\033[H");

    else if (cmd == "reload") {
        Serial.println("[Config] Reloading /config.ini...");
        loadConfig("/config.ini");
        Serial.println("[Config] Reload complete.");
    }

    else if (cmd == "help") {
        Serial.println("Commands:");
        Serial.println(": ls / list / dir      - List files");
        Serial.println(": cat <file>           - View file");
        Serial.println(": edit <file>          - Edit file");
        Serial.println(": rm <file>            - Delete file");
        Serial.println(": cp <src> <dst>       - Copy file");
        Serial.println(": mv <src> <dst>       - Rename file");
        Serial.println(": touch <file>         - Create file");
        Serial.println(": echo text > file     - Write file");
        Serial.println(": config               - Show runtime config");
        Serial.println(": reload               - Reload config.ini");
        Serial.println(": clear / cls          - Clear screen");
        Serial.println(": help                 - Show this help");
    }

    else Serial.println("[?] Unknown command. Try : help");

    Serial.println();
    if (enableCommandHistory && command.length() > 0) {
        if (commandHistory.empty() || command != commandHistory.back()) {
            if (commandHistory.size() >= maxHistoryEntries)
                commandHistory.erase(commandHistory.begin());
            commandHistory.push_back(command);
        }
        historyIndex = commandHistory.size();
    }
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
        case ':': handleColonCommand(inputBuffer); break;
        case '>': handleBroadcastCommand(inputBuffer); break;
        case '@': handleDirectMessageCommand(inputBuffer); break;
        case '#': handleSSHCommand(inputBuffer); break;
        default: Serial.println("[?] Unknown input prefix."); break;
    }

if (activePrefix == '#' && activeSSHNode.length()) {
    Serial.printf("# [%s] ", activeSSHNode.c_str());
} else {
    Serial.print(activePrefix);
    Serial.print(" ");
}

}


// =============================
// Main Serial Loop
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



String lastDMTarget = ""; // 💡 Put this at the top of the file

void handleDirectMessageCommand(const String& input) {
    int spaceIdx = input.indexOf(' ');
    if (spaceIdx == -1) {
        Serial.println("[DM] Usage: @<node> <message> or @r <reply>");
        return;
    }

    String rawTarget = input.substring(0, spaceIdx);
    String message = input.substring(spaceIdx + 1);

    // Strip the '@'
    if (!rawTarget.startsWith("@")) {
        Serial.println("[DM] Invalid format. Use @<node> <msg>");
        return;
    }

    String target = rawTarget.substring(1);  // Strip '@'

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


void handleSSHCommand(const String& input) {
    String trimmed = input;
    trimmed.trim();

    // 🚀 If not in session, treat this line as node ID to open session
    if (activeSSHNode == "") {
        if (trimmed.length() == 0) {
            Serial.println("[SSH] No session. Use: #<node>");
            return;
        }

        activeSSHNode = trimmed;
        Serial.printf("[SSH] Session opened with %s\n", activeSSHNode.c_str());
        return;
    }

    // ✅ Already in session → send command
    UMFrame frame;
    frame.type = UMFrame::SHELL;
    frame.from = nodeId;
    frame.to = activeSSHNode;
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = "cmd";
    frame.payload = input;

    String encoded = frame.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

    if (input == "exit") {
        Serial.printf("[SSH] Session closed with %s\n", activeSSHNode.c_str());
        activeSSHNode = "";
        activePrefix = '>';
    }
}

