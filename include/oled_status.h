// include/oled_status.h
#ifndef OLED_STATUS_H
#define OLED_STATUS_H

#include <Arduino.h>

void initOLED(const char* deviceName, const char* connectionType);
void updateOLEDStatus(const char* statusMessage);
void updateOLEDStats(); // <-- THIS WAS MISSING!!

#endif
