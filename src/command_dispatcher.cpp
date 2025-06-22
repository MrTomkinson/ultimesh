/**
 * @file command_dispatcher.cpp
 * @brief Dispatches shell/SSH commands by mapping command strings to function pointers using JSON config.
 */

#include "command_dispatcher.h"
#include "command_handlers.h"
#include "json_loader.h"
#include "debug_commands.h"

#include <ArduinoJson.h>
#include <map>

extern JsonArray loadedCommandList;

typedef void (*CommandHandler)(const String&, Print*);
std::map<String, CommandHandler> handlerMap;

// ✅ Called once at startup to register available command implementations
void initCommandDispatcher() {
    handlerMap["cmd_ls"]          = cmd_ls;
    handlerMap["cmd_ls_detailed"] = cmd_ls_detailed;
    handlerMap["cmd_cat"]         = cmd_cat;
    handlerMap["cmd_rm"]          = cmd_rm;
    handlerMap["cmd_edit"]        = cmd_edit;
    handlerMap["cmd_touch"]       = cmd_touch;
    handlerMap["cmd_mv"]          = cmd_mv;
    handlerMap["cmd_cp"]          = cmd_cp;
    handlerMap["cmd_echo"]        = cmd_echo;
    handlerMap["cmd_clear"]       = cmd_clear;
    handlerMap["cmd_help"]        = cmd_help;
    handlerMap["cmd_config"]      = cmd_config;
    handlerMap["cmd_reload"]      = cmd_reload;
    handlerMap["cmd_tokens"]      = cmd_tokens;
    handlerMap["cmd_top"]         = cmd_top;
    handlerMap["cmd_exit"]        = cmd_exit;
    handlerMap["cmd_reboot"]      = cmd_reboot;
    handlerMap["cmd_ping"]        = cmd_ping;
    handlerMap["cmd_version"]     = cmd_version;
    handlerMap["cmd_whoami"]      = cmd_whoami;

    // 🧪 Register test/debug commands last
    registerDebugCommands();
}

// ✅ Called any time a command string is to be executed
bool executeCommandByJson(const String& input, const String& mode, Print* out) {
    String trimmed = input;
    trimmed.trim();
    if (trimmed.length() == 0) {
        out->println("[!] Empty command");
        return false;
    }

    int spaceIndex = trimmed.indexOf(' ');
    String command = (spaceIndex == -1) ? trimmed : trimmed.substring(0, spaceIndex);
    String args = (spaceIndex == -1) ? "" : trimmed.substring(spaceIndex + 1);

    for (JsonObject cmdObj : loadedCommandList) {
        String name = cmdObj["name"] | "";
        bool nameMatch = (name == command);
        bool aliasMatch = false;

        if (!nameMatch && cmdObj.containsKey("aliases")) {
            for (JsonVariant alias : cmdObj["aliases"].as<JsonArray>()) {
                if (alias.as<String>() == command) {
                    aliasMatch = true;
                    break;
                }
            }
        }

        if (nameMatch || aliasMatch) {
            // ✅ Confirm allowed in current mode
            if (cmdObj.containsKey("modes")) {
                JsonArray modes = cmdObj["modes"].as<JsonArray>();
                bool allowed = false;
                for (JsonVariant m : modes) {
                    if (m.as<String>() == mode) {
                        allowed = true;
                        break;
                    }
                }
                if (!allowed) {
                    out->printf("[!] Command '%s' not allowed in mode '%s'\n", command.c_str(), mode.c_str());
                    return false;
                }
            }

            // ✅ Dispatch to registered handler
            String exec = cmdObj["exec"] | "";
            if (handlerMap.count(exec)) {
                handlerMap[exec](args, out);
                return true;
            } else {
                out->printf("[!] No handler found for exec: '%s'\n", exec.c_str());
                return false;
            }
        }
    }

    out->printf("[?] Unknown command: %s\n", command.c_str());
    return false;
}
