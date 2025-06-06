#pragma once
#include <Arduino.h>

struct UMFrame {
    enum FrameType {
        DATA,
        ACK,
        CONTROL,
        END,
        BROADCAST,
        DIRECT_MSG,
        SSH,
        SHELL,
        RESP
    };

    FrameType type;
    String from;
    String to;
    uint8_t chunkNumber;
    uint8_t totalChunks;
    String filename;
    String payload;

    String encode() const;
    static UMFrame parse(const String& raw);
    static FrameType parseType(char c);
    static char typeToChar(FrameType type);
};
