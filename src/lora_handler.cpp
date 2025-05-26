#include "lora_handler.h"
#include <SPI.h>
#include <LoRa.h>
#include "token_codec.h"

#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26
#define LORA_BAND 915E6

void initLoRa() {
  Serial.println("[LoRa] Initializing...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("[LoRa] LoRa init failed!");
    while (true);
  }
  Serial.println("[LoRa] Initialized");
}

void sendMessage(const String& encodedMessage, const char* recipient) {
  // TODO: actual recipient logic
  LoRa.beginPacket();
  LoRa.print(encodedMessage);
  LoRa.endPacket();
}

void sendMessage(const String& encodedMessage) {
  sendMessage(encodedMessage, nullptr);  // broadcast for now
}

void handleLoRaTraffic() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    Serial.print("[LoRa] Packet received: ");
    Serial.println(incoming);

    String decoded = decodeTokens(incoming);
    Serial.print("[LoRa] Decoded: ");
    Serial.println(decoded);
  }
}
