#ifndef TOKEN_CODEC_H
#define TOKEN_CODEC_H

#include <Arduino.h>
#include <map>

bool loadTokenMap(const char* path = "/tokens/tokens_shell.txt");
String encodeText(const String& raw);
String decodeTokens(const String& compressed);

#endif
