/**
 * @file frame_utils.cpp
 * @brief Implements string/byte conversion helpers used in frame encoding/decoding.
 */

#include "frame_utils.h"

std::vector<uint8_t> toBytes(const String& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

String fromBytes(const std::vector<uint8_t>& v) {
    return String(reinterpret_cast<const char*>(v.data()), v.size());
}
