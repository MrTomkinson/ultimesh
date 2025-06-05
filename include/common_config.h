#pragma once
#include <Arduino.h>

// System Identity
extern String nodeId;
extern String userName;
extern bool locationEnabled;

// LoRa
extern float frequency;
extern int spreadFactor;
extern int txPower;

// Message Behavior
extern int sendDelayMs;
extern int maxPacketSize;

// Shell/UI
extern char defaultShellMode;
extern bool showTop;
extern String loraNodeId;
extern String startupCommand;
extern String defaultTokenMap;
extern bool editorHelp;
extern bool enableCommandHistory;
extern bool enableTabCompletion;
extern int maxHistoryEntries;
extern int maxTabResults;

// OLED
extern int oledDisplayDuration;
extern bool oledFlashOnLora;
extern bool stickyTopEnabled;
// SSH / Transfer
extern bool sshCompressionEnabled;
extern bool sshTokenEnabled;
extern int sshMaxChunkSize;
extern int sshChunkDelayMs;

// Protocol
extern int protocolMaxPacketSize;
extern int sshPayloadLimit;
extern int chatPayloadLimit;
extern int protocolChunkDelay;
extern int maxRetries;
