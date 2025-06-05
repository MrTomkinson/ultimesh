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


  ## Console Modes (v0.b2+)

Ultimesh now supports modular console modes:

- `:` **Shell** – Local commands like `ls`, `cat`, `edit`, etc.
- `>` **Broadcast** – Send public LoRa messages to all nodes (`> hello`)
- `@` **Direct Message (DM)** – Send private messages to specific nodes
  - Usage: `@<node> <message>`
  - Quick reply: `@r <message>` to reply to last DM sender
- `#` **SSH** – Open and interact with a remote shell
  - Usage: `#<node>` (e.g., `#fred001`)
  - After session is open: just type commands
  - `exit` closes the session

Switch modes by typing the symbol alone:

:       → Shell mode
>       → Broadcast
@       → Direct Message
#       → SSH


  ## Version: 0.B1

### OLED Behavior

- Incoming LoRa messages cause the OLED to switch to message view
- Message is displayed with sender name and content
- OLED reverts to `top` after a delay set in `oled_lora_display_ms`

### LoRa Commands

Enter these in `>` mode:

| Command | Description |
|--------|-------------|
| `> hello world` | Broadcast message |
| `> /dm bob001 ping` | Direct message to `bob001` |

### Shell Navigation

| Mode | Command |
|------|---------|
| OLED Pager | `: pager` |
| OLED Top | `: top` |
| Launch Editor | `: edit /config.ini` |



   📡 Roadmap (Next)

Lynx-style text browser for internal web/BBS


CF-card support for true directory structure


🧠 Project Philosophy
"Everything old is new again."
ULTIMESH is for people who want control, visibility, and function even when the world is dark. ASCII-first. Serial-native. Mesh-ready.

© 2025 MrTomkinson / oldtechlife.com
