// serial_shell.h
#ifndef SERIAL_SHELL_H
#define SERIAL_SHELL_H

#include <Arduino.h>

void handleShell();
void executeShellCommand(const String& cmd);
void handleColonCommand(const String& command);
void handleChatCommand(const String& command);
void handleWebCommand(const String& command);
void handleSerialShell();    // ✅ Add this if missing!
void processInputChar(char c);


#endif
