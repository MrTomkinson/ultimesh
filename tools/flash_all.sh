#!/bin/bash
# ULTIMESH Mass Flash Script
# Flashes multiple ESP32 devices automatically.

# List of serial ports
ports=(
  /dev/ttyUSB0
  /dev/ttyUSB1
  /dev/ttyUSB2
  /dev/ttyUSB3
)

# Paths to built firmware files
BOOTLOADER=".pio/build/heltec_wifi_lora_32/bootloader.bin"
PARTITIONS=".pio/build/heltec_wifi_lora_32/partitions.bin"
FIRMWARE=".pio/build/heltec_wifi_lora_32/firmware.bin"

# Path to esptool
ESPTOOL=~/esptool-venv/bin/esptool.py

echo "🚀 Starting batch flashing of ESP32 units..."

for port in "${ports[@]}"; do
  echo "⚡ Flashing $port..."

  $ESPTOOL --chip esp32 --port $port --baud 921600 write_flash \
    0x1000   "$BOOTLOADER" \
    0x8000   "$PARTITIONS" \
    0x10000  "$FIRMWARE"

  echo "✅ Flash done for $port"
  echo "--------------------------------------"
done

echo "🏁 Batch flashing complete!"
