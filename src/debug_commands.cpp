/**
 * @file debug_commands.cpp
 * @brief Contains debug/testing command handlers for shell and SSH simulation.
 */

#include "debug_commands.h"
//#include "command_dispatcher.h"
//#include "common_config.h"
//#include "chunked_transfer.h"
//#include "frame_utils.h"
//#include "UMFrame.h"
//#include "ssh_session.h"
//#include <Arduino.h>

//extern std::map<String, void(*)(const String&, Print*)> handlerMap;

/**
 * @brief Sends a manual response message to the configured node.
 */
//void debug_resp(const String& args, Print* out) {
//    String to = "NODE001";  // Change as needed
//    String result = args;

//    UMFrame resp;
//    resp.type = UMFrame::RESP;
//    resp.from = toBytes(nodeId);
//    resp.to = toBytes(to);
//    resp.chunkNumber = 1;
//    resp.totalChunks = 1;
//    resp.filename = toBytes("resp");
//    resp.payload = toBytes(result);

//    out->println("[debug_resp] Sending manual RESP payload...");
//    sendLargePayload(to, "resp", result, UMFrame::RESP);
//}

/**
 * @brief Simulates receiving an SSH command frame.
 */
//void debug_ssh(const String& args, Print* out) {
//    out->println("[debug_ssh] Simulating SSH command dispatch...");

//    UMFrame sshFrame;
//    sshFrame.type = UMFrame::SSH;
//    sshFrame.from = toBytes("fred001");
//    sshFrame.to = toBytes(nodeId);  // Our own node
//    sshFrame.chunkNumber = 1;
//    sshFrame.totalChunks = 1;
//    sshFrame.filename = toBytes("debug");
//    sshFrame.payload = toBytes(args.length() ? args : "whoami");

//    handleSSHFrame(sshFrame);  // Call real SSH dispatcher
//}

/**
 * @brief Called by dispatcher to register these debug/test commands.
 */
void registerDebugCommands() {
//    handlerMap["debug_resp"] = debug_resp;
//    handlerMap["debug_ssh"]  = debug_ssh;
}
