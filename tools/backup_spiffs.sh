#!/bin/bash

# ULTIMESH - Backup only SPIFFS partition
# Usage:
#   ./backup_spiffs.sh /dev/ttyUSB0 backup_spiffs.bin

if [ $# -ne 2 ]; then
    echo "Usage: $0 /dev/ttyUSBx backup_spiffs.bin"
    exit 1
fi

DEVICE="$1"
OUTPUT="$2"

echo "📥 Backing up SPIFFS from $DEVICE to $OUTPUT..."
# 0x210000 offset, 0x5F0000 size (adjust if partition layout changes)
esptool.py --chip esp32 --port "$DEVICE" read_flash 0x210000 0x5F0000 "$OUTPUT"
echo "✅ SPIFFS backup complete: $OUTPUT"
