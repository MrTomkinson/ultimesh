#pragma once
#include <Arduino.h>
#include "UMFrame.h"

void sendLargePayload(const String& to, const String& filename, const String& payload, UMFrame::FrameType type);
void handleIncomingFrame(const UMFrame& frame);
