#!/bin/bash

# ULTIMESH - Restore only SPIFFS partition
# Usage:
#   ./restore_spiffs.sh /dev/ttyUSB0 backup_spiffs.bin

if [ $# -ne 2 ]; then
    echo "Usage: $0 /dev/ttyUSBx backup_spiffs.bin"
    exit 1
fi

DEVICE="$1"
INPUT="$2"

echo "📤 Restoring SPIFFS to $DEVICE from $INPUT..."
esptool.py --chip esp32 --port "$DEVICE" write_flash 0x210000 "$INPUT"
echo "✅ SPIFFS restore complete: $INPUT"
