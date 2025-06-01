#include "common_config.h"
#include "config_loader.h"

// === Feature Toggles ===
bool enableCommandHistory = true;
bool enableTabCompletion  = true;
int  maxHistoryEntries    = 10;
int  maxTabResults        = 8;

// === System Identity ===
String nodeId           = "NODE001";
String userName         = "User";
String defaultMode      = "pager";
String defaultTokenMap  = "/tokens_shell.txt";
String shellMode        = ":";
String startupCommand   = "";

// === LoRa Radio Settings ===
int   sendDelayMs       = 250;
int   maxPacketSize     = 192;
int   spreadFactor      = 7;
int   txPower           = 17;
float frequency         = 915E6;
bool  locationEnabled   = false;

// === OLED / UI ===
int  oledDisplayDuration = 5000;
bool showHelpOnStart     = true;
bool flashOnLoRa         = true;
bool showTop             = true;
bool stickyTopEnabled = false;  // Or true if you want top mode by default

void updateCommonSettings() {
    nodeId           = getConfigValue("node_id", nodeId);
    userName         = getConfigValue("user_name", userName);
    defaultMode      = getConfigValue("default_mode", defaultMode);
    defaultTokenMap  = getConfigValue("default_token_map", defaultTokenMap);
    shellMode        = getConfigValue("shell_mode", shellMode);
    startupCommand   = getConfigValue("startup_command", startupCommand);

    sendDelayMs      = getConfigInt("send_delay_ms", sendDelayMs);
    maxPacketSize    = getConfigInt("max_packet_size", maxPacketSize);
    spreadFactor     = getConfigInt("spread_factor", spreadFactor);
    txPower          = getConfigInt("tx_power", txPower);
    frequency        = getConfigFloat("frequency", frequency);
    locationEnabled  = getConfigBool("location_enabled", locationEnabled);

    oledDisplayDuration = getConfigInt("oled_lora_display_ms", oledDisplayDuration);
    showHelpOnStart     = getConfigBool("show_help_on_start", showHelpOnStart);
    flashOnLoRa         = getConfigBool("oled_flash_on_lora", flashOnLoRa);
    showTop             = getConfigBool("show_top", showTop);

    enableCommandHistory = getConfigBool("enable_command_history", enableCommandHistory);
    enableTabCompletion  = getConfigBool("enable_tab_completion", enableTabCompletion);
    maxHistoryEntries    = getConfigInt("max_history_entries", maxHistoryEntries);
    maxTabResults        = getConfigInt("max_tab_results", maxTabResults);
}

void printConfig() {
    Serial.println("[Config Dump]");
    for (const std::pair<String, String>& pair : runtimeConfig) {
        Serial.printf(" %-24s = %s\n", pair.first.c_str(), pair.second.c_str());
    }
}
