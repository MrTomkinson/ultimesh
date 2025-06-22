#pragma once
#include "StringBufferPrint.h"
#include <Arduino.h>
#include <vector>

std::vector<String> splitString(const String& input, char delimiter);

inline std::vector<uint8_t> toBytes(const String& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

inline String fromBytes(const std::vector<uint8_t>& data) {
    return String(reinterpret_cast<const char*>(data.data()), data.size());
}

