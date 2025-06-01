#pragma once

#include <Arduino.h>

void initLoRa();
void handleLoRaTraffic();
void sendLoRaMessage(const String& message, const String& to = "BCAST");
