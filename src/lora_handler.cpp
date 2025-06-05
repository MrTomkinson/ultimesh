#include "lora_handler.h"
#include "oled_status.h"
#include "config_loader.h"
#include "common_config.h"
#include "UMFrame.h"
#include "ssh_session.h"
#include <LoRa.h>
#include "serial_shell.h"
void initLoRa() {
    LoRa.setPins(18, 14, 26);
    if (!LoRa.begin((long)frequency)) {
        Serial.println("[LoRa] Init failed!");
    } else {
        Serial.println("[LoRa] Init OK");
    }

    LoRa.setSpreadingFactor(spreadFactor);
    LoRa.setTxPower(txPower);
}

void handleLoRaTraffic() {
    int packetSize = LoRa.parsePacket();
    if (!packetSize) return;

    String raw = "";
    while (LoRa.available()) raw += (char)LoRa.read();
    raw.trim();

    UMFrame frame = UMFrame::parse(raw);
    Serial.printf("[Parsed] From %s → To %s | Type=%d | File=%s\n",
                  frame.from.c_str(), frame.to.c_str(), frame.type, frame.filename.c_str());

    // 🔍 Ensure only intended recipients process the frame
    String toLower = frame.to;
    String selfLower = nodeId;
    toLower.toLowerCase();
    selfLower.toLowerCase();

    if (toLower != "all" && toLower != selfLower) return;

    if (frame.totalChunks > 1) {
        Serial.printf("[LoRa] Chunk %d/%d from %s\n", frame.chunkNumber, frame.totalChunks, frame.from.c_str());
    }

    switch (frame.type) {
        case UMFrame::DIRECT_MSG:
            showLoRaMessage("DM", frame.payload, oledDisplayDuration);
            Serial.printf("[DM] %s\n", frame.payload.c_str());
                lastDMFrom = frame.from;  // <- Track the last DM sender
            break;

        case UMFrame::BROADCAST:
            showLoRaMessage("LoRa", frame.payload, oledDisplayDuration);
            Serial.printf("[BROADCAST] %s\n", frame.payload.c_str());
            break;

        case UMFrame::SSH:
            if (frame.filename == "cmd") {
                handleRemoteSSHCommand(frame.from, frame.payload);
            } else {
                Serial.println(frame.payload);  // Only show responses
            }
            break;

        case UMFrame::SHELL:
        case UMFrame::CONTROL:
            if (frame.payload.startsWith("[SSH:")) {
                int idx = frame.payload.indexOf(']');
                if (idx != -1) {
                    String from = frame.payload.substring(5, idx);
                    String cmd = frame.payload.substring(idx + 2);
                    handleRemoteSSHCommand(from, cmd);
                }
            } else {
                handleRemoteSSHCommand(frame.from, frame.payload);
            }
            break;

        default:
            Serial.printf("[LoRa] Unknown type: %d\n", frame.type);
            break;
    }
}



void sendLoRaMessage(const String& message, const String& target) {
    UMFrame frame;
    frame.type = (target.equalsIgnoreCase("BCAST") || target.equalsIgnoreCase("ALL"))
                    ? UMFrame::BROADCAST
                    : UMFrame::DIRECT_MSG;

    frame.from = nodeId;
    frame.to = (frame.type == UMFrame::BROADCAST) ? "ALL" : target;
    frame.chunkNumber = 1;
    frame.totalChunks = 1;
    frame.filename = "-";
    frame.payload = message;

    String encoded = frame.encode();
    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

    Serial.printf("[LoRa] Sent to %s: %s\n", frame.to.c_str(), frame.payload.c_str());
}
