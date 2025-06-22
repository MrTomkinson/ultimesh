#include "lora_handler.h"
#include "common_config.h"
#include "oled_status.h"
#include "UMFrame.h"
#include "chunked_transfer.h"
#include "common_utils.h"

#include <LoRa.h>
    const int ssPin = 18;
    const int resetPin = 14;
    const int dio0Pin = 26;

// Initialize LoRa transceiver
void initLoRa() {
    LoRa.setPins(18, 14, 26);  // NSS, RST, DIO0

    Serial.printf("[LoRa] Initializing with freq: %lu Hz\n", frequency);
Serial.printf("[LoRa] Pins: ss=%d reset=%d dio0=%d\n", ssPin, resetPin, dio0Pin);

    if (!LoRa.begin((long)frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }

    LoRa.setSpreadingFactor(spreadFactor);
    LoRa.setTxPower(txPower);
    Serial.println("LoRa initialized.");
}

// Handle incoming LoRa traffic
void handleLoRaTraffic() {
    int packetSize = LoRa.parsePacket();
    if (packetSize == 0) return;

    std::vector<uint8_t> rawData;
    while (LoRa.available()) {
        rawData.push_back(LoRa.read());
    }

    UMFrame frame;
    if (!UMFrame::parse(rawData.data(), rawData.size(), frame)) {
        Serial.println("[LoRa] Failed to parse frame.");
        return;
    }

    String selfLower = nodeId;
    selfLower.toLowerCase();

   String target = fromBytes(frame.to);
    target.toLowerCase();

    if (target != "all" && target != selfLower) {
        Serial.printf("[LoRa] Not intended for this node: %s\n", target.c_str());
        return;
    }

String filename = fromBytes(frame.filename);
String payload = fromBytes(frame.payload);


    handleIncomingFrame(frame);
}

// Send simple LoRa message (BROADCAST, DIRECT_MSG)
void sendLoRaMessage(const String& toStr, const String& message) {
    UMFrame frame;
    frame.type = toStr == "ALL" ? UMFrame::BROADCAST : UMFrame::DIRECT_MSG;
    frame.from = std::vector<uint8_t>(nodeId.begin(), nodeId.end());
    frame.to = std::vector<uint8_t>(toStr.begin(), toStr.end());
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = {'-'};
    frame.payload = std::vector<uint8_t>(message.begin(), message.end());

    sendLargePayload(toStr, "-", message, frame.type);
}

// Send full encoded frame over LoRa
void sendFrame(const UMFrame& frame) {
    std::vector<uint8_t> encoded;
    if (frame.encode(encoded)) {
        LoRa.beginPacket();
        LoRa.write(encoded.data(), encoded.size());
        LoRa.endPacket();
    } else {
        Serial.println("[sendFrame] ❌ Encoding failed");
    }
}

