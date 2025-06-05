#pragma once

#include <Arduino.h>

extern String lastDMFrom;
void initLoRa();
void handleLoRaTraffic();
void sendLoRaMessage(const String& message, const String& target);
void sendBroadcastMessage(const String& payload);
void sendShellCommand(const String& to, const String& command);
