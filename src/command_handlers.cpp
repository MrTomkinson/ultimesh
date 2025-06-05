#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "command_handlers.h"
#include "common_config.h"
#include "config_loader.h"
#include "file_storage.h"
#include "text_editor.h"
#include "oled_status.h"
#include "reverse_token_map.h"
#include "json_loader.h"
extern char activePrefix;
extern String nodeId;
extern bool stickyTopEnabled;

// Simple handler macros
#define NOT_IMPLEMENTED(name) Serial.printf("[!] %s not implemented yet\n", name)

void cmd_ls(const String& args) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("       %-24s %6d bytes\n", file.name(), file.size());
        file = root.openNextFile();
    }
}

void cmd_ls_detailed(const String& args) {
    // Placeholder - same as ls for now
    cmd_ls(args);
}

void cmd_cat(const String& args) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    String content = readFile(path.c_str());
    if (content.isEmpty()) Serial.println("[!] File not found or empty");
    else Serial.println(content);
}

void cmd_rm(const String& args) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    if (SPIFFS.remove(path)) Serial.printf("[✓] Deleted: %s\n", path.c_str());
    else Serial.printf("[!] Failed to delete: %s\n", path.c_str());
}

void cmd_edit(const String& args) {
    String file = args;
    if (!file.startsWith("/")) file = "/" + file;
    launchTextEditor(file);
    activePrefix = ':';  // Return to shell after
}

void cmd_touch(const String& args) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    File f = SPIFFS.open(path, FILE_WRITE);
    if (!f) {
        Serial.println("[!] Failed to create file");
        return;
    }
    f.close();
    Serial.printf("[+] Created: %s\n", path.c_str());
}

void cmd_mv(const String& args) {
    int split = args.indexOf(' ');
    if (split < 1) {
        Serial.println("[mv] Usage: mv <source> <dest>");
        return;
    }

    String src = args.substring(0, split);
    String dst = args.substring(split + 1);
    if (!src.startsWith("/")) src = "/" + src;
    if (!dst.startsWith("/")) dst = "/" + dst;

    if (!SPIFFS.rename(src, dst)) {
        Serial.println("[!] Rename failed.");
    } else {
        Serial.printf("[✓] Renamed to %s\n", dst.c_str());
    }
}

void cmd_cp(const String& args) {
    int split = args.indexOf(' ');
    if (split < 1) {
        Serial.println("[cp] Usage: cp <source> <dest>");
        return;
    }

    String src = args.substring(0, split);
    String dst = args.substring(split + 1);
    if (!src.startsWith("/")) src = "/" + src;
    if (!dst.startsWith("/")) dst = "/" + dst;

    String content = readFile(src.c_str());
    if (!writeFile(dst.c_str(), content)) {
        Serial.println("[!] Copy failed.");
    } else {
        Serial.printf("[✓] Copied to %s\n", dst.c_str());
    }
}

void cmd_echo(const String& args) {
    Serial.println(args);
}

void cmd_clear(const String& args) {
    Serial.print("\033[2J\033[H");
}

void cmd_help(const String&) {
    Serial.println(F("Available commands:"));
    for (const auto& entry : loadedCommandList) {
        String name = entry["name"] | "unknown";
        String desc = entry["desc"] | "";
        String aliasLine = "";

        if (entry.containsKey("aliases")) {
            JsonArray aliases = entry["aliases"].as<JsonArray>();
            for (JsonVariant v : aliases) {
                aliasLine += v.as<String>() + ", ";
            }
            if (aliasLine.length() > 2)
                aliasLine.remove(aliasLine.length() - 2);  // remove trailing comma/space
        }

        Serial.printf("  %-12s - %s", name.c_str(), desc.c_str());
        if (aliasLine.length()) {
            Serial.printf(" (aliases: %s)", aliasLine.c_str());
        }
        Serial.println();
    }
}


void cmd_config(const String& args) {
    printConfig();
}

void cmd_reload(const String& args) {
    Serial.println("[Config] Reloading /config.ini...");
    loadConfig("/config.ini");
    Serial.println("[Config] Reload complete.");
}

void cmd_tokens(const String& args) {
    for (auto &pair : reverseMap) {
        Serial.printf("%u = %s\n", pair.second, pair.first.c_str());
    }
}

void cmd_top(const String& args) {
    currentMode = MODE_TOP;
    returnMode = MODE_TOP;
}

void cmd_exit(const String& args) {
    Serial.println("[SSH] Session closed.");
    // This will be handled in shell logic
}

void cmd_reboot(const String& args) {
    Serial.println("[System] Rebooting...");
    ESP.restart();
}

void cmd_ping(const String& args) {
    Serial.printf("[ping] Not implemented: would ping node %s\n", args.c_str());
}

void cmd_version(const String& args) {
    Serial.printf("Ultimesh Firmware v%s\n", firmwareVersion);
}

void cmd_whoami(const String& args) {
    Serial.printf("Node ID: %s\n", nodeId.c_str());
}
