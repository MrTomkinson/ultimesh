#pragma once
#include <Arduino.h>
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"

// These functions wrap heatshrink compression for Arduino Strings
String compressData(const String& input);
String decompressData(const String& input);
