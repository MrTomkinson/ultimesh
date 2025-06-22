/**
 * @file ssh_session.cpp
 * @brief Handles incoming SSH-style remote command execution frames.
 */
#include "config_loader.h"  // not common_config.h

#include "UMFrame.h"
#include "chunked_transfer.h"
#include "frame_utils.h"
#include "command_dispatcher.h"

#include <Arduino.h>

// Helper: Implements Print to collect output into a String
class PrintBuffer : public Print {
public:
    String buf;
    size_t write(uint8_t c) override {
        buf += (char)c;
        return 1;
    }
    size_t write(const uint8_t* data, size_t len) override {
        buf.concat((const char*)data, len);
        return len;
    }
    String str() const { return buf; }
};

void handleSSHFrame(const UMFrame &frame) {
    String targetId = getConfigValue("node_id");
    if (frame.to != toBytes(targetId) && frame.to != toBytes("ALL")) {
        return;
    }

    // Convert payload vector to command string
    String command;
    for (uint8_t b : frame.payload) {
        command += (char)b;
    }
    command.trim();

    PrintBuffer out;
    bool success = executeCommandByJson(command, "ssh", &out);
    String result = out.str();

    if (!success || result.length() == 0) {
        result = "[error] Command failed or returned nothing.";
    }

    int limit = getConfigInt("ssh_payload_limit", 190);
    if (result.length() > limit) {
        result = result.substring(0, limit);
    }

    // Convert frame.from (vector<uint8_t>) to String for sendLargePayload
    String fromStr;
    for (uint8_t b : frame.from) {
        fromStr += (char)b;
    }

    sendLargePayload(fromStr, "-", result, UMFrame::RESP);
}
