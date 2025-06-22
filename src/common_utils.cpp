#include "common_utils.h"

std::vector<String> splitString(const String& input, char delimiter) {
    std::vector<String> tokens;
    int start = 0;
    int end = input.indexOf(delimiter);

    while (end != -1) {
        tokens.push_back(input.substring(start, end));
        start = end + 1;
        end = input.indexOf(delimiter, start);
    }

    tokens.push_back(input.substring(start));
    return tokens;
}
String fromBytes(const std::vector<uint8_t>& data);
std::vector<uint8_t> toBytes(const String& str);
