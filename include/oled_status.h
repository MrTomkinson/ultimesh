#pragma once
#include <Arduino.h>

#ifndef OLED_STATUS_H
#define OLED_STATUS_H

enum OledMode {
    MODE_TOP,
    MODE_MESSAGE
};

void initOLED(const char* deviceName, const char* connectionType);
void drawTopScreen();
void showLoRaMessage(const String& sender, const String& msg, int durationMs);
void handleOLED();

extern OledMode currentMode;
extern OledMode returnMode;

#endif
