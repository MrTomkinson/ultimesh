#include "UMFrame.h"
#include "config_loader.h"  // for splitString
#include <vector>
#include "UMFrame.h"
#include "config_loader.h"

UMFrame UMFrame::parse(const String& raw) {
    UMFrame frame;

    // Serial.println("[DEBUG] Raw LoRa message: " + raw);

    if (!raw.startsWith("[") || raw.indexOf("]") == -1) {
        frame.payload = raw;
        frame.type = DATA;
        return frame;
    }

    int headerEnd = raw.indexOf(']');
    String header = raw.substring(1, headerEnd);
    frame.payload = raw.substring(headerEnd + 1);
    frame.payload.trim();

    auto parts = splitString(header, '|');
    while (parts.size() < 5) parts.push_back("");  // pad with blanks

    // safer parsing
    if (parts[0].length() == 0) {
        Serial.println("[UMFrame] Malformed type header. Defaulting to DATA.");
        frame.type = DATA;
    } else {
        frame.type = parseType(parts[0][0]);
    }

    frame.from = parts[1];
    frame.to = parts[2];

    auto chunkParts = splitString(parts[3], '/');
    frame.chunkNumber = chunkParts.size() == 2 ? chunkParts[0].toInt() : 1;
    frame.totalChunks = chunkParts.size() == 2 ? chunkParts[1].toInt() : 1;

    frame.filename = parts[4];

    //Serial.printf("[UMFrame] Parsed: type=%d from=%s to=%s file=%s\n",
       //           frame.type, frame.from.c_str(), frame.to.c_str(), frame.filename.c_str());

    return frame;
}


String UMFrame::encode() const {
    char typeChar = typeToChar(type);
    String header = "[" + String(typeChar) + "|" + from + "|" + to + "|" +
                    String(chunkNumber) + "/" + String(totalChunks) + "|" +
                    (filename.length() ? filename : "-") + "]";
    return header + " " + payload;
}

UMFrame::FrameType UMFrame::parseType(char c) {
    switch (c) {
        case 'D': return DATA;
        case 'A': return ACK;
        case 'C': return CONTROL;
        case 'E': return END;
        case 'B': return BROADCAST;
        case 'M': return DIRECT_MSG;
        case 'S': return SSH;
        case 'H': return SHELL;
        default:  return DATA;
    }
}

char UMFrame::typeToChar(FrameType type) {
    switch (type) {
        case DATA:        return 'D';
        case ACK:         return 'A';
        case CONTROL:     return 'C';
        case END:         return 'E';
        case BROADCAST:   return 'B';
        case DIRECT_MSG:  return 'M';
        case SSH:         return 'S';
        case SHELL:       return 'H';
        default:          return 'D';
    }
}
