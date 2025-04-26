#!/bin/bash
# ULTIMESH Mass Backup Script
# Backs up multiple ESP32 devices automatically.

# List of serial ports to scan
ports=(
  /dev/ttyUSB0
  /dev/ttyUSB1
  /dev/ttyUSB2
  /dev/ttyUSB3
)

# Backup folder
backup_dir="backups"
mkdir -p $backup_dir

# Path to correct esptool
ESPTOOL=~/esptool-venv/bin/esptool.py

echo "📥 Starting batch backup of ESP32 units..."

for port in "${ports[@]}"; do
  outfile="$backup_dir/$(basename $port)_backup.bin"
  echo "📦 Backing up from $port to $outfile..."
  $ESPTOOL --chip esp32 --port $port read_flash 0x00000 0x800000 "$outfile"
  echo "✅ Done: $outfile"
  echo "--------------------------------------"
done

echo "🏁 Batch backup complete!"
