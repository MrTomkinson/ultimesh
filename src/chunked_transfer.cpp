#include "chunked_transfer.h"
#include "compression_utils.h"
#include "UMFrame.h"
#include "command_dispatcher.h"
#include "common_utils.h"
#include "ssh_session.h"
#include "common_config.h"      // ✅ uses global config vars
#include "lora_handler.h"

#include <Arduino.h>
#include <vector>
#include <map>

static std::map<String, std::vector<UMFrame>> reassemblyMap;

std::vector<UMFrame> splitIntoChunks(const String& from, const String& to, const String& filename, const String& message, UMFrame::FrameType type) {
    std::vector<UMFrame> chunks;
    std::vector<uint8_t> compressed = compressData(toBytes(message));
    size_t totalSize = compressed.size();
    size_t maxPayload = sshPayloadLimit;  // ✅ use global config var
    size_t totalChunks = (totalSize + maxPayload - 1) / maxPayload;

    Serial.printf("[splitIntoChunks] Message from %s → %s [%s], compressed: %d bytes, %d chunk(s)\n",
        from.c_str(), to.c_str(), filename.c_str(), totalSize, totalChunks);

    for (size_t i = 0; i < totalChunks; ++i) {
        UMFrame chunk;
        chunk.type = type;
        chunk.from = toBytes(from);
        chunk.to = toBytes(to);
        chunk.chunkNumber = i + 1;
        chunk.totalChunks = totalChunks;
        chunk.filename = toBytes(filename);

        size_t start = i * maxPayload;
        size_t end = std::min(start + maxPayload, totalSize);
        chunk.payload = std::vector<uint8_t>(compressed.begin() + start, compressed.begin() + end);
        chunks.push_back(chunk);
    }

    return chunks;
}

bool reassembleChunks(const UMFrame& frame, UMFrame& outFrame) {
    String key = fromBytes(frame.from) + "|" + fromBytes(frame.filename);
    reassemblyMap[key].push_back(frame);
    auto& chunks = reassemblyMap[key];
    if (chunks.size() < frame.totalChunks) return false;

    std::sort(chunks.begin(), chunks.end(), [](const UMFrame& a, const UMFrame& b) {
        return a.chunkNumber < b.chunkNumber;
    });

    std::vector<uint8_t> combined;
    for (const auto& chunk : chunks) {
        combined.insert(combined.end(), chunk.payload.begin(), chunk.payload.end());
    }

    std::vector<uint8_t> decompressed = decompressData(combined);

    outFrame = chunks[0];
    outFrame.payload = decompressed;
    outFrame.chunkNumber = 1;
    outFrame.totalChunks = 1;

    reassemblyMap.erase(key);
    return true;
}

void sendAck(const UMFrame& frame) {
    UMFrame ack;
    ack.type = UMFrame::ACK;
    ack.from = frame.to;
    ack.to = frame.from;
    ack.chunkNumber = frame.chunkNumber;
    ack.totalChunks = 1;
    ack.filename = toBytes("ack");
    ack.payload = toBytes("ACK");

    Serial.printf("[sendAck] Sending ACK to %s\n", fromBytes(ack.to).c_str());
    sendFrame(ack);
}

void dispatchPayload(const UMFrame& frame) {
    String from = fromBytes(frame.from);
    String to = fromBytes(frame.to);
    String payload = fromBytes(frame.payload);

Serial.printf("[dispatchPayload] RAW PAYLOAD: %s\n", payload.c_str());

    Serial.printf("[dispatchPayload] Frame Type: %d | From: %s → To: %s | Payload size: %d\n",
        frame.type, from.c_str(), to.c_str(), payload.length());

    switch (frame.type) {
        case UMFrame::DIRECT_MSG:
        case UMFrame::RESP:
            Serial.printf("[DM/RESP] From: %s → %s\n", from.c_str(), to.c_str());
            Serial.println(payload);
            break;

        case UMFrame::SHELL:
            Serial.printf("[SHELL] Executing: %s\n", payload.c_str());
            executeCommandByJson(payload, "shell");
            break;

        case UMFrame::SSH:
            handleSSHFrame(frame);
            break;

        case UMFrame::BROADCAST:
            Serial.printf("[BROADCAST] %s\n", payload.c_str());
            break;

        case UMFrame::ACK:
            Serial.printf("[ACK] ACK received from %s\n", from.c_str());
            break;

        default:
            Serial.printf("[UNHANDLED] Type %d from %s\n", frame.type, from.c_str());
            break;
    }
}

void handleIncomingFrame(const UMFrame& frame) {
    String target = fromBytes(frame.to);
    target.trim();

    // 🔁 TEMP: bypass address match
    // String local = nodeId;
    // local.trim();
    // if (target != local) return;

    Serial.printf("[handleIncomingFrame] Frame to: %s, from: %s, type: %d\n",
                  target.c_str(), fromBytes(frame.from).c_str(), frame.type);

    if (frame.totalChunks == 1) {
        dispatchPayload(frame);
        if (frame.type != UMFrame::ACK) {
            sendAck(frame);
        }
    } else {
        UMFrame full;
        if (reassembleChunks(frame, full)) {
            dispatchPayload(full);
            sendAck(frame);
        }
    }
}


void sendLargePayload(const String& to, const String& filename, const String& message, UMFrame::FrameType type) {
    Serial.printf("[sendLargePayload] %s → %s | File: %s | Type: %d | Bytes: %d\n",
        nodeId.c_str(), to.c_str(), filename.c_str(), type, message.length());

    auto chunks = splitIntoChunks(nodeId, to, filename, message, type);

    for (const auto& chunk : chunks) {
        sendFrame(chunk);
        Serial.printf("[sendLargePayload] Sent chunk %d/%d, waiting %dms for ACK...\n",
                      chunk.chunkNumber, chunk.totalChunks, protocolChunkDelay);
        delay(protocolChunkDelay);  // ✅ from config
    }

    Serial.printf("[sendLargePayload] ✅ All %d chunk(s) sent to %s\n", chunks.size(), to.c_str());
}
