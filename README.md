# ULTIMESH

ULTIMESH is a retro-inspired ESP32 LoRa mesh terminal built for offline, portable, and emergency communications. Designed to run headless and serial-attached, it provides a modular shell, tokenized LoRa chat (WIP), file tools, OLED system view, and an onboard editable config system.
Turn ANY digital computer or dumb terminal into a fully self contained communication device. Allows for lora and wifi communication oner usb, serial and parallel. Access other nodes, internet or C/F mounted files on any system, anywhere.
---

## 📦 Features

- Dual-mode OLED display (pager + system monitor)
- Modular ASCII command shell with DOS/Linux aliases
- Editable token map & persistent SPIFFS storage
- Minimal built-in file editor (nano/dosedit style)
- Tab-completion (commands and files)
- Configurable runtime values (via `/config.ini`)
- Flat virtual filesystem for SPIFFS, CF support planned
- Live mode switching: shell `:`, lora `>`, web `/`, BBS `~`

---

## 🔧 Build Instructions

```bash
cd ~/ultimesh
source .platformio-env/bin/activate

# Upload firmware
platformio run --target upload

# Upload SPIFFS data (/data/)
platformio run --target uploadfs

# Monitor output
platformio device monitor --baud 115200

💻 Shell Commands
Command	Description
ls / list / dir	List SPIFFS files
cat <file>	View file contents
edit <file>	Launch file editor
rm <file>	Delete file
cp <src> <dst>	Copy file
mv <src> <dst>	Rename/move file
touch <file>	Create empty file
echo text > file	Write text to file
config	Show runtime config (/config.ini)
clear / cls	Clear screen
tokens	Show loaded token map
top	Toggle OLED system monitor
help	Show this help screen

🛠 Editor Controls
Ctrl+S – Save

Ctrl+A – Save As

Ctrl+N – New File

Ctrl+Q – Quit

Ctrl+I – Help Menu

Arrow keys – Move cursor

Tab – Command/File autocomplete

📁 SPIFFS Layout
Flat file structure only — directories simulated for compatibility (, not working well). All files referenced must be uploaded to /data/ before flashing:

/data/
  test.txt
   config.ini
   tokens_shell.txt
   tokens_sys.txt
   etc

   Files will appear as flat /tokens_xxx.txt entries inside SPIFFS.

📡 Roadmap (Next)
LoRa DM command support (> dm node message)

Load/save multiple token maps dynamically

Lynx-style text browser for internal web/BBS

Configurable editor keys (via config file)

CF-card support for true directory structure

Background LoRa listener buffer with log view

🧠 Project Philosophy
"Everything old is new again."
ULTIMESH is for people who want control, visibility, and function even when the world is dark. ASCII-first. Serial-native. Mesh-ready.

© 2025 MrTomkinson / oldtechlife.com
