#include "ssh_session.h"
#include "file_storage.h"
#include "lora_handler.h"
#include "token_codec.h"
#include "common_config.h"
#include "config_loader.h"
#include "UMFrame.h"
#include "command_dispatcher.h"
#include <LoRa.h>
#include <SPIFFS.h>

// 🧠 Print buffer class to capture Serial output
class BufferCapture : public Print {
  public:
    String buffer;
    size_t write(uint8_t c) override {
        buffer += (char)c;
        return 1;
    }
};

class StringPrinter : public Print {
public:
    StringPrinter(String& buffer) : _buffer(buffer) {}
    size_t write(uint8_t c) override {
        _buffer += (char)c;
        return 1;
    }
    size_t write(const uint8_t* buffer, size_t size) override {
        _buffer += String((const char*)buffer).substring(0, size);
        return size;
    }
private:
    String& _buffer;
};


void sendSSHResponse(const String& toNode, const String& responseText) {
    size_t maxLen = 192;
    size_t start = 0;

    while (start < responseText.length()) {
        String chunk = responseText.substring(start, start + maxLen);

        UMFrame frame;
        frame.type = UMFrame::RESP;
        frame.from = nodeId;
        frame.to = toNode;
        frame.chunkNumber = 1;
        frame.totalChunks = 1;
        frame.filename = "resp";
        frame.payload = chunk;

        String encoded = frame.encode();
        LoRa.beginPacket();
        LoRa.print(encoded);
        LoRa.endPacket();

        start += maxLen;
        delay(400);  // 🕓 Prevent packet overlap
    }
}

void handleRemoteSSHCommand(const String& fromNode, const String& rawCommand) {
    //Serial.printf("[RemoteSSH] Raw command from %s: %s\n", fromNode.c_str(), rawCommand.c_str());

    String command = rawCommand;
    command.trim();

    // 🧼 Strip [SSH:NODEID]
    if (command.startsWith("[SSH:")) {
        int end = command.indexOf(']');
        if (end > 0) {
            command = command.substring(end + 1);
            command.trim();
        }
    }

    //Serial.printf("[RemoteSSH] Cleaned command: %s\n", command.c_str());

    // 💥 Special-case for exit
    if (command == "exit") {
        sendSSHResponse(fromNode, "[SSH] Session closed.");
        return;
    }

    // 🔁 Capture output instead of printing to Serial
    BufferCapture capture;
Serial.printf("[SSH] Executing remote command: %s\n", command.c_str());
    //Serial.println("[DEBUG] About to call executeCommandByJson() with redirected output:");
    Serial.println(command);

    bool success = executeCommandByJson(command, "ssh", &capture);  // <-- You must update this function to accept Print*

    if (capture.buffer.length() > 0) {
        sendSSHResponse(fromNode, capture.buffer);
    } else if (!success) {
        sendSSHResponse(fromNode, "[?] Unknown command: " + command);
    } else {
        sendSSHResponse(fromNode, "[✓] Executed: " + command);
    }
}
