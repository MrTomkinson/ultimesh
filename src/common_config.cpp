#include "common_config.h"
#include "config_loader.h"
#include <map>
const char* firmwareVersion = "0.1.0";
// --- System Identity ---
String nodeId = "NODE001";
String userName = "Tom";
bool locationEnabled = false;

// --- LoRa ---
float frequency = 915E6;
int spreadFactor = 7;
int txPower = 17;

// --- Message Behavior ---
int sendDelayMs = 250;
int maxPacketSize = 192;

// --- Shell/UI ---
char defaultShellMode = ':'; // or config string parsed
bool showTop = true;
//String loraNodeId = "ULTI-01";
String startupCommand = "top";
String defaultTokenMap = "/tokens_shell.txt";
bool editorHelp = true;
bool enableCommandHistory = true;
bool enableTabCompletion = true;
int maxHistoryEntries = 10;
int maxTabResults = 8;

// --- OLED ---
int oledDisplayDuration = 5000;
bool oledFlashOnLora = true;
bool stickyTopEnabled = true;  // or false, based on default
// --- SSH / Transfer ---
bool sshCompressionEnabled = false;
bool sshTokenEnabled = false;
int sshMaxChunkSize = 192;
int sshChunkDelayMs = 500;

// --- Protocol ---
int protocolMaxPacketSize = 220;
int sshPayloadLimit = 180;
//int chatPayloadLimit = 200;
int protocolChunkDelay = 500;
int maxRetries = 3;

void updateCommonSettings() {
    nodeId = getConfigValue("node_id", nodeId);
    userName = getConfigValue("user_name", userName);
    locationEnabled = getConfigBool("location_enabled", locationEnabled);

    frequency = getConfigFloat("frequency", frequency);
    spreadFactor = getConfigInt("spread_factor", spreadFactor);
    txPower = getConfigInt("tx_power", txPower);

    sendDelayMs = getConfigInt("send_delay_ms", sendDelayMs);
    maxPacketSize = getConfigInt("max_packet_size", maxPacketSize);

    defaultShellMode = getConfigValue("shell_mode", ":")[0];
    showTop = getConfigBool("show_top", showTop);
    //loraNodeId = getConfigValue("lora_node_id", loraNodeId);
    startupCommand = getConfigValue("startup_command", startupCommand);
    defaultTokenMap = getConfigValue("default_token_map", defaultTokenMap);
    editorHelp = getConfigBool("editor_help", editorHelp);
    enableCommandHistory = getConfigBool("enable_command_history", enableCommandHistory);
    enableTabCompletion = getConfigBool("enable_tab_completion", enableTabCompletion);
    maxHistoryEntries = getConfigInt("max_history_entries", maxHistoryEntries);
    maxTabResults = getConfigInt("max_tab_results", maxTabResults);

    oledDisplayDuration = getConfigInt("oled_lora_display_ms", oledDisplayDuration);
    oledFlashOnLora = getConfigBool("oled_flash_on_lora", oledFlashOnLora);

    sshCompressionEnabled = getConfigBool("ssh_compression_enabled", sshCompressionEnabled);
    sshTokenEnabled = getConfigBool("ssh_token_enabled", sshTokenEnabled);
    sshMaxChunkSize = getConfigInt("ssh_max_chunk_size", sshMaxChunkSize);
    sshChunkDelayMs = getConfigInt("ssh_chunk_delay_ms", sshChunkDelayMs);

    protocolMaxPacketSize = getConfigInt("max_packet_size", protocolMaxPacketSize);
    sshPayloadLimit = getConfigInt("ssh_payload_limit", sshPayloadLimit);
    //chatPayloadLimit = getConfigInt("chat_payload_limit", chatPayloadLimit);
    protocolChunkDelay = getConfigInt("chunk_delay_ms", protocolChunkDelay);
    maxRetries = getConfigInt("max_retries", maxRetries);
}
