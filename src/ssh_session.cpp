#include "ssh_session.h"
#include "file_storage.h"
#include "lora_handler.h"
#include "token_codec.h"
#include "common_config.h"
#include "config_loader.h"  // for SSH config vars
#include "UMFrame.h"     // ✅ Needed for UMFrame struct
#include <LoRa.h>        // ✅ Needed for LoRa.beginPacket() and .print()
#include <SPIFFS.h>

void sendSSHResponse(const String& toNode, const String& responseText) {
    size_t maxLen = 192;
    size_t start = 0;

    while (start < responseText.length()) {
        String chunk = responseText.substring(start, start + maxLen);

        UMFrame frame;
        frame.type = UMFrame::SSH;
        frame.from = nodeId;
        frame.to = toNode;
        frame.chunkNumber = 1;
        frame.totalChunks = 1;
        frame.filename = "resp";  // ✅ NEW: Mark this as a response
        frame.payload = chunk;

        String encoded = frame.encode();
        LoRa.beginPacket();
        LoRa.print(encoded);
        LoRa.endPacket();

        start += maxLen;
        delay(400);
    }
}



void handleRemoteSSHCommand(const String& fromNode, const String& rawCommand) {
    String cmd = rawCommand;
    cmd.trim();

    // Extract command from payload: "[SSH:NODEID] command"
    if (cmd.startsWith("[SSH:")) {
        int end = cmd.indexOf(']');
        if (end > 0) {
            cmd = cmd.substring(end + 1);  // Drop "[SSH:XYZ]"
            cmd.trim();
        }
    }

    if (cmd == "help") {
        sendSSHResponse(fromNode,
            "Remote SSH Help:\n"
            " ls         - List files\n"
            " cat <file> - View file\n"
            " rm <file>  - Delete file\n"
            " help       - Show this help\n");
        return;
    }

if (cmd == "ls") {
    if (!SPIFFS.begin(true)) {
        sendSSHResponse(fromNode, "[!] SPIFFS mount failed");
        return;
    }

    String output = "";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        output += String(file.name()) + " (" + file.size() + " bytes)\n";
        file = root.openNextFile();
    }

    sendSSHResponse(fromNode, output.isEmpty() ? "[!] No files" : output);
    return;
}

    if (cmd.startsWith("cat ")) {
        String path = cmd.substring(4);
        if (!path.startsWith("/")) path = "/" + path;
        String content = readFile(path.c_str());
        sendSSHResponse(fromNode, content.isEmpty() ? "[!] File not found or empty" : content);
        return;
    }

    if (cmd.startsWith("rm ")) {
        String path = cmd.substring(3);
        if (!path.startsWith("/")) path = "/" + path;
        bool success = SPIFFS.remove(path);
        sendSSHResponse(fromNode, success ? "[✓] Deleted" : "[!] Delete failed");
        return;
    }

    if (cmd == "exit") {
        sendSSHResponse(fromNode, "[SSH] Session closed.");
        return;
    }

    // Unknown command
    sendSSHResponse(fromNode, "[?] Unknown command: " + cmd);
}

