#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"
#include "lora_handler.h"
#include "oled_status.h"
#include "text_editor.h"
#include "common_config.h"
#include "config_loader.h"

#include <SPIFFS.h>
#include <FS.h>
#include <vector>
#include <map>

#define ENABLE_COMMAND_HISTORY (enableCommandHistory)
#define ENABLE_TAB_COMPLETION  (enableTabCompletion)

extern std::map<String, uint16_t> reverseMap;
extern bool stickyTopEnabled;

void sendLoRaMessage(const String& message, const String& target);

String inputBuffer = "";
char activePrefix = ':';  // Default shell mode

std::vector<String> commandHistory;
int historyIndex = -1;

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

// LoRa command handler
#include "common_config.h"  // for nodeId

void handleGreaterThanCommand(const String& inputLine) {
    String input = inputLine;
    input.trim();

    if (input == "?" || input == "help") {
        Serial.println("LoRa Commands:");
        Serial.println("> <message>           - Broadcast message");
        Serial.println("> /dm <nodeID> <msg>  - Direct message");
        return;
    }

    if (input.startsWith("/dm ")) {
        int space = input.indexOf(' ', 4);
        if (space > 0) {
            String target = input.substring(4, space);
            String msg = input.substring(space + 1);

            Serial.printf("[SHELL] Sending DM to '%s': %s\n", target.c_str(), msg.c_str());

            // Correctly wrap in DM format
            String packet = "[DM:" + target + "] " + msg;
            sendLoRaMessage(packet, "BCAST");  // DM packets still go to all nodes
            return;
        } else {
            Serial.println("[!] Usage: /dm <nodeID> <message>");
            return;
        }
    }

    // Default: Broadcast message
    Serial.printf("[SHELL] Sending broadcast: %s\n", input.c_str());
    sendLoRaMessage(input, "BCAST");
}



// Tab completion helpers
std::vector<String> getAllCommands() {
    return {
        "ls", "list", "dir", "cat", "rm", "edit", "cp", "mv", "touch", "echo",
        "tokens", "top", "clear", "cls", "help", "config"
    };
}

std::vector<String> getMatchingCompletions(const String& prefix, int limit = 8) {
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
            if (!f) {
                Serial.printf("[!] Failed to open %s\n", path.c_str());
            } else {
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

    else if (cmd == "config") {
        printConfig();
    }

    else if (cmd == "cat" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        String content = readFile(file.c_str());
        if (content.isEmpty()) {
            Serial.println("[!] Not found or empty.");
        } else {
            Serial.println(content);
        }
    }

    else if (cmd == "rm" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        if (SPIFFS.remove(file)) {
            Serial.printf("[✓] Deleted: %s\n", file.c_str());
        } else {
            Serial.printf("[!] Failed to delete: %s\n", file.c_str());
        }
    }

    else if (cmd == "edit" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        launchTextEditor(file);
        activePrefix = ':'; // restore mode
    }

    else if (cmd == "cp" && tokens.size() > 2) {
        String src = tokens[1], dst = tokens[2];
        if (!src.startsWith("/")) src = "/" + src;
        if (!dst.startsWith("/")) dst = "/" + dst;
        File in = SPIFFS.open(src, FILE_READ);
        File out = SPIFFS.open(dst, FILE_WRITE);
        if (!in || !out) {
            Serial.println("[!] Copy failed.");
        } else {
            while (in.available()) out.write(in.read());
            Serial.printf("[✓] Copied %s → %s\n", src.c_str(), dst.c_str());
        }
        in.close();
        out.close();
    }

    else if (cmd == "mv" && tokens.size() > 2) {
        String src = tokens[1], dst = tokens[2];
        if (!src.startsWith("/")) src = "/" + src;
        if (!dst.startsWith("/")) dst = "/" + dst;
        if (SPIFFS.rename(src, dst)) {
            Serial.printf("[✓] Renamed to: %s\n", dst.c_str());
        } else {
            Serial.println("[!] Rename failed.");
        }
    }

    else if (cmd == "touch" && tokens.size() > 1) {
        String file = tokens[1];
        if (!file.startsWith("/")) file = "/" + file;
        File f = SPIFFS.open(file, FILE_WRITE);
        if (f) {
            f.close();
            Serial.printf("[✓] Created: %s\n", file.c_str());
        } else {
            Serial.println("[!] Failed to create.");
        }
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

    else if (cmd == "clear" || cmd == "cls") {
        Serial.print("\033[2J\033[H");
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
        Serial.println(": clear / cls          - Clear screen");
        Serial.println(": help                 - Show this help");
    }

    else {
        Serial.println("[?] Unknown command. Try : help");
    }

    Serial.println();
    if (enableCommandHistory && command.length() > 0) {
        if (commandHistory.empty() || command != commandHistory.back()) {
            if (commandHistory.size() >= maxHistoryEntries) {
                commandHistory.erase(commandHistory.begin());
            }
            commandHistory.push_back(command);
        }
        historyIndex = commandHistory.size();
    }
}

void handleInputLine(const String& line) {
    String trimmed = line;
    trimmed.trim();

    // Detect shell mode prefix (e.g., "> message")
    if (trimmed.length() >= 2 &&
        (trimmed[0] == ':' || trimmed[0] == '>' || trimmed[0] == '/' || trimmed[0] == '~') &&
        trimmed[1] == ' ') {
        activePrefix = trimmed[0];
        inputBuffer = trimmed.substring(2);
    } else {
        inputBuffer = trimmed;
    }

    switch (activePrefix) {
        case ':': handleColonCommand(inputBuffer); break;
        case '>': handleGreaterThanCommand(inputBuffer); break;
        case '/': Serial.println("[Web mode coming]"); break;
        case '~': Serial.println("[BBS mode coming]"); break;
    }

    Serial.print(activePrefix);
    Serial.print(" ");
}

void handleSerialShell() {
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
