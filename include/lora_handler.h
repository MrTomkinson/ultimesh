#pragma once

#include <Arduino.h>
#include "UMFrame.h"

void sendFrame(const UMFrame& frame); 
extern String lastDMFrom;
void initLoRa();
void handleLoRaTraffic();
void sendLoRaMessage(const String& message, const String& target);
void sendBroadcastMessage(const String& payload);
void sendShellCommand(const String& to, const String& command);
