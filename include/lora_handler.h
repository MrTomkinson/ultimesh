#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <Arduino.h>

void initLoRa();
void handleLoRaTraffic();
void sendMessage(const String& msg);  // ✅ <-- This line was missing!

#endif
