#include "lora_handler.h"
#include "oled_status.h"
#include "config_loader.h"
#include "common_config.h"
#include "UMFrame.h"
#include "ssh_session.h"
#include <LoRa.h>
#include "serial_shell.h"
#include "command_dispatcher.h"
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
    //Serial.printf("[LoRa RAW PACKET] \"%s\"\n", raw.c_str());

    raw.trim();

       if (raw.length() == 0) {
        Serial.println("[LoRa] ⚠ Empty packet received. Ignored.");
        return;
       }

    UMFrame frame = UMFrame::parse(raw);
    //Serial.printf("[UMFrame] type=%d from=%s to=%s file=%s chunk=%d/%d\n",
              frame.type,
              frame.from.c_str(), frame.to.c_str(), frame.filename.c_str(),
              frame.chunkNumber, frame.totalChunks;
// Serial.printf("[UMFrame] payload: %s\n", frame.payload.c_str());

//Serial.printf("[Parsed] From %s → To %s | Type=%c (%d) | File=%s\n",
              frame.from.c_str(), frame.to.c_str(),
              UMFrame::typeToChar((UMFrame::FrameType)frame.type),
              frame.type,
              frame.filename.c_str();
    // Ignore if not for us or a broadcast
   String fromLower = frame.from; fromLower.toLowerCase();
String toLower = frame.to; toLower.toLowerCase();
String selfLower = nodeId; selfLower.toLowerCase();

// 💣 Prevent self-processing
if (fromLower == selfLower) {
    Serial.println("[LoRa] Ignoring self-sent frame.");
    return;
}


//Serial.printf("[LoRa] Check routing: to=%s | self=%s\n", toLower.c_str(), selfLower.c_str());

// 💡 Continue only if message is addressed to this node or ALL
if (toLower != "all" && toLower != selfLower) return;

    switch (frame.type) {
        case UMFrame::BROADCAST:
            Serial.printf("[BROADCAST] %s\n", frame.payload.c_str());
            break;

        case UMFrame::DIRECT_MSG:
            Serial.printf("[DM] %s\n", frame.payload.c_str());
            lastDMFrom = frame.from;
            break;

case UMFrame::SHELL: {
    if (frame.to != nodeId) {
        Serial.printf("[LoRa] Ignoring frame not addressed to this node. To: %s | Me: %s\n", frame.to.c_str(), nodeId.c_str());
        break;
    }

    String command = frame.payload;

    Serial.printf("[SSH] Received command from %s: %s\n", frame.from.c_str(), command.c_str());

    // ✅ Correct handler call — this includes [SSH:..] cleanup
    handleRemoteSSHCommand(frame.from, frame.payload);

    break;
}


      case UMFrame::RESP: {
         // Serial.printf("[LoRa] Incoming frame of type RESP from %s\n", frame.from.c_str());
       //   Serial.printf("[LoRa] RESP filename: %s\n", frame.filename.c_str());
      //   Serial.printf("[LoRa] RESP payload: %s\n", frame.payload.c_str());

          if (frame.filename == "resp") {
             Serial.printf("[RESP from %s] %s\n", frame.from.c_str(), frame.payload.c_str());
          }
          break;
      }

      default:
          Serial.printf("[LoRa] ⚠ Unknown frame type: %c\n", UMFrame::typeToChar((UMFrame::FrameType)frame.type));
          break;

    }
}


void sendLoRaMessage(const String& message, const String& target) {
    if (message.length() == 0) {
        Serial.println("[LoRa] ⚠ Skipped empty message.");
        return;
    }

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
    if (encoded.length() == 0) {
        Serial.println("[LoRa] ⚠ Encoded message was empty. Aborting send.");
        return;
    }

    LoRa.beginPacket();
    LoRa.print(encoded);
    LoRa.endPacket();

   // Serial.printf("[LoRa] Sent to %s: %s\n", frame.to.c_str(), frame.payload.c_str());
}

