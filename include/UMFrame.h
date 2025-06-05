#pragma once
#include <Arduino.h>

class UMFrame {
public:
    enum FrameType {
        DATA,
        ACK,
        CONTROL,
        END,
        BROADCAST,
        DIRECT_MSG,
        SSH,
        SHELL
    };

    FrameType type;
    String from;
    String to;
    uint16_t chunkNumber;
    uint16_t totalChunks;
    String filename;
    String payload;

    static UMFrame parse(const String& raw);
    String encode() const;

private:
    static FrameType parseType(char c);
    static char typeToChar(FrameType type);
};
