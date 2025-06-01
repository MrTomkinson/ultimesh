#include "lora_handler.h"
#include "oled_status.h"
#include "config_loader.h"
#include "common_config.h"
#include <LoRa.h>

void initLoRa() {
    LoRa.setPins(18, 14, 26);
    if (!LoRa.begin((long)frequency)) {
        Serial.println("[LoRa] Init failed!");

} else {
    Serial.println("[LoRa] Init successful.");
    }

    LoRa.setSpreadingFactor(spreadFactor);
    LoRa.setTxPower(txPower);
    Serial.println("[LoRa] Init successful.");
}

void sendLoRaMessage(const String& message, const String& target) {
    String packet;
    if (target == "BCAST") {
        packet = message;
    } else {
        packet = "[DM:" + target + "] " + message;
    }

    Serial.printf("[LoRa] Sending: %s\n", packet.c_str());

    LoRa.beginPacket();
    LoRa.print(packet);
    LoRa.endPacket();
}

void handleLoRaTraffic() {
    int packetSize = LoRa.parsePacket();
    if (!packetSize) return;

    Serial.printf("[LoRa] Packet size: %d\n", packetSize);

    String message = "";
    while (LoRa.available()) {
        message += (char)LoRa.read();
    }

    Serial.printf("[LoRa] Raw message: %s\n", message.c_str());

    // Validate: printable characters only
    bool printable = true;
    for (char c : message) {
        if ((c < 32 || c > 126) && c != '\n' && c != '\r') {
            printable = false;
            break;
        }
    }

    if (!printable) {
        Serial.println("[LoRa] Unrecognized packet (non-printable):");
        Serial.println(message);
        return;
    }

    // DM parsing
    if (message.startsWith("[DM:")) {
        int start = message.indexOf(":") + 1;
        int end = message.indexOf("]");
        if (start > 0 && end > start) {
            String target = message.substring(start, end);
            String content = message.substring(end + 2);

            Serial.printf("[LoRa] DM received: target=%s | content=%s | nodeId=%s\n",
                          target.c_str(), content.c_str(), nodeId.c_str());

            if (target == nodeId) {
                Serial.println("[LoRa] DM matches this node, displaying...");
                showLoRaMessage("DM", content, oledDisplayDuration);
            } else {
                Serial.println("[LoRa] DM not for this node, ignoring.");
            }
            return;
        }
    }

    // Broadcast
    Serial.printf("[LoRa] Broadcast: %s\n", message.c_str());
    showLoRaMessage("LoRa", message, oledDisplayDuration);
}
