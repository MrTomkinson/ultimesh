#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#pragma once
#include <Arduino.h>
#include <map>

#include <Print.h>  // Required for Print*
typedef void (*CommandHandler)(const String&, Print*);
extern std::map<String, CommandHandler> handlerMap;

void initCommandDispatcher();
bool executeCommandByJson(const String& input, const String& mode, Print* out = &Serial);
String dispatchCommand(const String& input);

#endif
