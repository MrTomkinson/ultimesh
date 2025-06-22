/**
 * @file frame_utils.h
 * @brief Utility functions to convert between String and byte vector formats.
 *
 * These functions are used to encode/decode `String` values to/from `std::vector<uint8_t>`,
 * commonly used in UMFrame structures for network communication.
 */

#pragma once
#include <Arduino.h>
#include <vector>

/**
 * @brief Convert a String into a byte vector.
 * @param s Input string.
 * @return Vector of bytes.
 */
std::vector<uint8_t> toBytes(const String& s);

/**
 * @brief Convert a byte vector back into a String.
 * @param v Vector of bytes.
 * @return String representation.
 */
String fromBytes(const std::vector<uint8_t>& v);
