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
#define NOT_IMPLEMENTED(name) out->printf("[!] %s not implemented yet\n", name)

void cmd_ls(const String& args, Print* out) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        out->printf("       %-24s %6d bytes\n", file.name(), file.size());
        file = root.openNextFile();
    }
}

void cmd_ls_detailed(const String& args, Print* out) {
    // Placeholder - same as ls for now
    cmd_ls(args, out);
}

void cmd_cat(const String& args, Print* out) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    String content = readFile(path.c_str());
    if (content.isEmpty()) out->println("[!] File not found or empty");
    else out->println(content);
}

void cmd_rm(const String& args, Print* out) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    if (SPIFFS.remove(path)) out->printf("[✓] Deleted: %s\n", path.c_str());
    else out->printf("[!] Failed to delete: %s\n", path.c_str());
}

void cmd_edit(const String& args, Print* out) {
    String file = args;
    if (!file.startsWith("/")) file = "/" + file;
    launchTextEditor(file);
    activePrefix = ':';  // Return to shell after
}

void cmd_touch(const String& args, Print* out) {
    String path = args;
    if (!path.startsWith("/")) path = "/" + path;
    File f = SPIFFS.open(path, FILE_WRITE);
    if (!f) {
        out->println("[!] Failed to create file");
        return;
    }
    f.close();
    out->printf("[+] Created: %s\n", path.c_str());
}

void cmd_mv(const String& args, Print* out) {
    int split = args.indexOf(' ');
    if (split < 1) {
        out->println("[mv] Usage: mv <source> <dest>");
        return;
    }

    String src = args.substring(0, split);
    String dst = args.substring(split + 1);
    if (!src.startsWith("/")) src = "/" + src;
    if (!dst.startsWith("/")) dst = "/" + dst;

    if (!SPIFFS.rename(src, dst)) {
        out->println("[!] Rename failed.");
    } else {
        out->printf("[✓] Renamed to %s\n", dst.c_str());
    }
}

void cmd_cp(const String& args, Print* out) {
    int split = args.indexOf(' ');
    if (split < 1) {
        out->println("[cp] Usage: cp <source> <dest>");
        return;
    }

    String src = args.substring(0, split);
    String dst = args.substring(split + 1);
    if (!src.startsWith("/")) src = "/" + src;
    if (!dst.startsWith("/")) dst = "/" + dst;

    String content = readFile(src.c_str());
    if (!writeFile(dst.c_str(), content)) {
        out->println("[!] Copy failed.");
    } else {
        out->printf("[✓] Copied to %s\n", dst.c_str());
    }
}

void cmd_echo(const String& args, Print* out) {
    out->println(args);
}

void cmd_clear(const String& args, Print* out) {
    out->print("\033[2J\033[H");
}

void cmd_help(const String&, Print* out) {
    out->println(F("Available commands:"));
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

        out->printf("  %-12s - %s", name.c_str(), desc.c_str());
        if (aliasLine.length()) {
            out->printf(" (aliases: %s)", aliasLine.c_str());
        }
        out->println();
    }
}

void cmd_config(const String& args, Print* out) {
   void printConfig(Print* out);
}

void cmd_reload(const String& args, Print* out) {
    out->println("[Config] Reloading /config.ini...");
    loadConfig("/config.ini");
    out->println("[Config] Reload complete.");
}

void cmd_tokens(const String& args, Print* out) {
    for (auto &pair : reverseMap) {
        out->printf("%u = %s\n", pair.second, pair.first.c_str());
    }
}

void cmd_top(const String& args, Print* out) {
    currentMode = MODE_TOP;
    returnMode = MODE_TOP;
}

void cmd_exit(const String& args, Print* out) {
    out->println("[SSH] Session closed.");
}

void cmd_reboot(const String& args, Print* out) {
    out->println("[System] Rebooting...");
    ESP.restart();
}

void cmd_ping(const String& args, Print* out) {
    out->printf("[ping] Not implemented: would ping node %s\n", args.c_str());
}

void cmd_version(const String& args, Print* out) {
    out->printf("Ultimesh Firmware v%s\n", firmwareVersion);
}

void cmd_whoami(const String& args, Print* out) {
    out->printf("Node ID: %s\n", nodeId.c_str());
}
