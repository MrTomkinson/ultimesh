#pragma once
#include <Arduino.h>
#include <vector>

// Compression using heatshrink
std::vector<uint8_t> compressData(const std::vector<uint8_t>& input);

// Decompression: raw pointer version (used by UMFrame::parse)
std::vector<uint8_t> decompressData(const uint8_t* input, size_t input_size);

// Decompression: vector wrapper (for convenience)
std::vector<uint8_t> decompressData(const std::vector<uint8_t>& input);
