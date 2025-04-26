#!/bin/bash

# ULTIMESH - Backup entire ESP32 flash to a file
# Usage:
#   ./backup_firmware.sh /dev/ttyUSB0 backup.bin

if [ $# -ne 2 ]; then
    echo "Usage: $0 /dev/ttyUSBx backup.bin"
    exit 1
fi

DEVICE="$1"
OUTPUT="$2"

echo "📥 Backing up full flash from $DEVICE to $OUTPUT..."
/home/main/esptool-venv/bin/esptool.py --chip esp32 --port "$DEVICE" read_flash 0x00000 0x800000 "$OUTPUT"
echo "✅ Backup complete: $OUTPUT"
