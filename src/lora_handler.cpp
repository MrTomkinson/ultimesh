#include "lora_handler.h"
#include <SPI.h>
#include <LoRa.h>

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
    Serial.println("❌ LoRa failed!");
    while (true);
  }
  Serial.println("[LoRa] Initialized");
}

void handleLoRaTraffic() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("[LoRa] Packet received: ");
    Serial.println(packetSize);
  }
}

// ✨ MISSING FUNCTION ADDED HERE
void sendMessage(const String& msg) {
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}
