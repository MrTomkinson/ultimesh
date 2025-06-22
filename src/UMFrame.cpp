#include "UMFrame.h"
#include "compression_utils.h"
#include "common_utils.h"

#include <sstream>
#include <cstring>

// Constructor
UMFrame::UMFrame() :
    type(DATA),
    chunkNumber(1),
    totalChunks(1),
    from(),
    to(),
    filename(),
    payload() {}


// Convert FrameType enum to a single character
char UMFrame::typeToChar(FrameType t) {
    switch (t) {
        case DATA: return 'D';
        case ACK: return 'K';
        case CONTROL: return 'C';
        case END: return 'E';
        case BROADCAST: return 'B';
        case DIRECT_MSG: return 'M';
        case SSH: return 'S';
        case SHELL: return 'H';
        case RESP: return 'R';
        case TRANSFER: return 'T';
        case CHUNK_ACK: return 'A';
        default: return '?';
    }
}

// Parse a single character into a FrameType
UMFrame::FrameType UMFrame::parseType(char c) {
    switch (c) {
        case 'D': return DATA;
        case 'K': return ACK;
        case 'C': return CONTROL;
        case 'E': return END;
        case 'B': return BROADCAST;
        case 'M': return DIRECT_MSG;
        case 'S': return SSH;
        case 'H': return SHELL;
        case 'R': return RESP;
        case 'T': return TRANSFER;
        case 'A': return CHUNK_ACK;
        default: return DATA;
    }
}


// Encode: header is plain text, payload is assumed pre-compressed
bool UMFrame::encode(std::vector<uint8_t>& output) const {
    output.clear();

    // Build header string
    String header = String(typeToChar(type)) + "|" +
                    fromBytes(from) + "|" +
                    fromBytes(to) + "|" +
                    String(chunkNumber) + "|" +
                    String(totalChunks) + "|" +
                    fromBytes(filename) + "|";

    Serial.printf("[UMFrame::encode] Raw header: %s\n", header.c_str());
    Serial.printf("[UMFrame::encode] Payload size: %d\n", payload.size());

    // Insert header into output
    output.insert(output.end(), header.begin(), header.end());

    // Append compressed payload
    output.insert(output.end(), payload.begin(), payload.end());

    Serial.printf("[UMFrame::encode] Compressed size: %d\n", output.size());
    return true;
}

// Parse: header is plain, then decompress payload
bool UMFrame::parse(const uint8_t* data, size_t len, UMFrame& out) {
    out = UMFrame(); // Reset

    std::vector<uint8_t> raw(data, data + len);

    // Find header end (position of the 6th '|')
    size_t pipeCount = 0;
    size_t headerEndIndex = 0;
    for (; headerEndIndex < raw.size(); ++headerEndIndex) {
        if (raw[headerEndIndex] == '|') {
            pipeCount++;
            if (pipeCount == 6) {
                headerEndIndex++;  // Move past last '|'
                break;
            }
        }
    }

    if (pipeCount < 6 || headerEndIndex >= raw.size()) {
        Serial.println("[UMFrame::parse] ❌ Malformed header");
        return false;
    }

    // Extract header string
    String header(reinterpret_cast<const char*>(raw.data()), headerEndIndex);
    std::vector<String> parts = splitString(header, '|');

    if (parts.size() < 6) {
        Serial.println("[UMFrame::parse] ❌ Header split failed");
        return false;
    }

    out.type = parseType(parts[0][0]);
    out.from = toBytes(parts[1]);
    out.to = toBytes(parts[2]);
    out.chunkNumber = parts[3].toInt();
    out.totalChunks = parts[4].toInt();
    out.filename = toBytes(parts[5]);

    // Extract compressed payload (remainder of data)
    std::vector<uint8_t> compressed(raw.begin() + headerEndIndex, raw.end());

    Serial.printf("[UMFrame::parse] Compressed input size: %d\n", (int)compressed.size());
    out.payload = decompressData(compressed);
    Serial.printf("[UMFrame::parse] Decompressed size: %d\n", (int)out.payload.size());

    String printableHeader = header;
    printableHeader.replace("\n", "\\n");
    printableHeader.replace("\r", "\\r");
    Serial.printf("[UMFrame::parse] Decompressed (printable): %s\n", printableHeader.c_str());

    String payloadPreview = fromBytes(out.payload);
    payloadPreview.replace("\n", "\\n");
    payloadPreview.replace("\r", "\\r");
    Serial.printf("[UMFrame::parse] Final payload: %s\n", payloadPreview.c_str());

    Serial.println("[UMFrame::parse] ✔ Frame parsed successfully.");
    return true;
}
