#include "serial_shell.h"
#include "file_storage.h"
#include "token_codec.h"

String inputBuffer = "";

void initSerialShell() {}

void handleSerialShell() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      Serial.println();
      Serial.print("> ");
      Serial.println(inputBuffer);

      if (inputBuffer.startsWith(": list")) {
        listFiles();
      }
      else if (inputBuffer.startsWith(": free")) {
        showFreeSpace();
      }

      inputBuffer = "";
    } else {
      inputBuffer += c;
      Serial.print(c); // Echo typed char
    }
  }
}
