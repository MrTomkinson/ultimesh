#!/bin/bash

PORT=$1

if [ -z "$PORT" ]; then
  echo "Usage: $0 /dev/ttyUSB0"
  exit 1
fi

esptool.py --chip esp32 --port $PORT --baud 921600 write_flash -z \
  0x1000   .pio/build/heltec_wifi_lora_32/bootloader.bin \
  0x8000   .pio/build/heltec_wifi_lora_32/partitions.bin \
  0x10000  .pio/build/heltec_wifi_lora_32/firmware.bin \
  0x290000 .pio/build/heltec_wifi_lora_32/spiffs.bin
