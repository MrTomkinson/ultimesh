#include "command_dispatcher.h"
#include "command_handlers.h"
#include "json_loader.h"  // <-- Holds the parsed command JSON
#include <ArduinoJson.h>
#include <map>
#include <vector>

typedef void (*CommandHandler)(const String&, Print*);
std::map<String, CommandHandler> handlerMap;

void initCommandDispatcher() {
    handlerMap["cmd_ls"]        = cmd_ls;
    handlerMap["cmd_ls_detailed"] = cmd_ls_detailed;
    handlerMap["cmd_cat"]       = cmd_cat;
    handlerMap["cmd_rm"]        = cmd_rm;
    handlerMap["cmd_edit"]      = cmd_edit;
    handlerMap["cmd_touch"]     = cmd_touch;
    handlerMap["cmd_mv"]        = cmd_mv;
    handlerMap["cmd_cp"]        = cmd_cp;
    handlerMap["cmd_echo"]      = cmd_echo;
    handlerMap["cmd_clear"]     = cmd_clear;
    handlerMap["cmd_help"]      = cmd_help;
    handlerMap["cmd_config"]    = cmd_config;
    handlerMap["cmd_reload"]    = cmd_reload;
    handlerMap["cmd_tokens"]    = cmd_tokens;
    handlerMap["cmd_top"]       = cmd_top;
    handlerMap["cmd_exit"]      = cmd_exit;
    handlerMap["cmd_reboot"]    = cmd_reboot;
    handlerMap["cmd_ping"]      = cmd_ping;
    handlerMap["cmd_version"]   = cmd_version;
    handlerMap["cmd_whoami"]    = cmd_whoami;
}

bool executeCommandByJson(const String& input, const String& mode, Print* out) {
    String rawCmd = input;
    rawCmd.trim();
    if (rawCmd.length() == 0) return false;

    int space = rawCmd.indexOf(' ');
    String cmd = (space > 0) ? rawCmd.substring(0, space) : rawCmd;
    String args = (space > 0) ? rawCmd.substring(space + 1) : "";

    for (const auto& entry : loadedCommandList) {
        String name = entry["name"];
        std::vector<String> aliases;

        if (entry.containsKey("aliases")) {
            for (JsonVariant v : entry["aliases"].as<JsonArray>()) {
                aliases.push_back(v.as<String>());
            }
        }

        bool nameMatch = (cmd == name);
        bool aliasMatch = std::any_of(aliases.begin(), aliases.end(), [&](const String& alias) {
            return alias == cmd;
        });

        if (nameMatch || aliasMatch) {
            // ✅ Check allowed modes
            if (entry.containsKey("modes")) {
                JsonArray modes = entry["modes"].as<JsonArray>();
                bool validMode = false;
                for (JsonVariant m : modes) {
                    if (m.as<String>() == mode) {
                        validMode = true;
                        break;
                    }
                }
                if (!validMode) {
                    out->println("[!] Command not allowed in this mode");
                    return false;
                }
            }

            // ✅ Find handler
            String exec = entry["exec"];
            if (handlerMap.count(exec)) {
                handlerMap[exec](args, out);
                return true;
            } else {
                out->printf("[!] Handler for '%s' not implemented\n", exec.c_str());
                return false;
            }
        }
    }

    out->printf("[?] Unknown command: %s\n", cmd.c_str());
    return false;
}
