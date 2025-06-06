#ifndef SSH_SESSION_H
#define SSH_SESSION_H

#include <Arduino.h>
extern String lastDMFrom;

// Handles an SSH command from a remote node (LoRa-wrapped)
void handleRemoteSSHCommand(const String& fromNode, const String& rawCommand);

// Sends a response (can be chunked) back to the requesting node
void sendSSHResponse(const String& toNode, const String& responseText);

#endif
