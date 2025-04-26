#!/bin/bash

# ULTIMESH - Burn firmware to one or multiple ESP32 devices
# Usage:
#   ./burn_all.sh             (auto-detect all)
#   ./burn_all.sh /dev/ttyUSB0 /dev/ttyUSB1 (manual select)
#   ./burn_all.sh --list       (list connected devices only)

BOOTLOADER=".pio/build/heltec_wifi_lora_32/bootloader.bin"
PARTITIONS=".pio/build/heltec_wifi_lora_32/partitions.bin"
FIRMWARE=".pio/build/heltec_wifi_lora_32/firmware.bin"

# Check files exist
for file in "$BOOTLOADER" "$PARTITIONS" "$FIRMWARE"; do
    if [ ! -f "$file" ]; then
        echo "❌ Missing required file: $file"
        exit 1
    fi
done

# List connected devices
if [[ "$1" == "--list" ]]; then
    echo "📋 Connected devices:"
    ls /dev/ttyUSB* 2>/dev/null || echo "❌ No devices found."
    exit 0
fi

# Auto-detect devices if none specified
if [ $# -eq 0 ]; then
    DEVICES=(/dev/ttyUSB*)
else
    DEVICES=("$@")
fi

# Flash each device
for dev in "${DEVICES[@]}"; do
    if [ ! -e "$dev" ]; then
        echo "⚠️  Skipping missing device: $dev"
        continue
    fi

    echo "🚀 Flashing $dev..."
    esptool.py --chip esp32 --port "$dev" --baud 921600 write_flash -z \
        0x1000 "$BOOTLOADER" \
        0x8000 "$PARTITIONS" \
        0x10000 "$FIRMWARE"

    echo "✅ Flash complete: $dev"
done
