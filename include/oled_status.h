#ifndef OLED_STATUS_H
#define OLED_STATUS_H

void initOLED(const char* deviceName, const char* connectionType);
void drawTopScreen();
void drawPagerScreen(const char* deviceName, const char* connectionType);

#endif
