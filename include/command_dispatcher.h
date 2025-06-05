#pragma once
#include <Arduino.h>
#include <map>

typedef void (*CommandHandler)(const String&);
extern std::map<String, CommandHandler> handlerMap;

void initCommandDispatcher();
bool executeCommandByJson(const String& rawCmd, const String& mode);
