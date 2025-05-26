
# ULTIMESH

ULTIMESH is a retro-inspired ESP32 LoRa mesh terminal built for portable, emergency, and offline communications. It supports ASCII-only interaction, modular shell commands, token-based message compression, and eventual web/BBS access. Built specifically for retro systems, it works over serial, parallel, and USB.

---

## 📦 Features

- Dual-mode OLED display: pager + system monitor (`top`)
- ASCII command shell with prefix-based interaction
- SPIFFS filesystem (6MB+) for token storage and files
- Tokenized LoRa chat with DM support (WIP)
- Modular interface support: USB, serial, LPT, CF-card (planned)
- Self-flashing, reflashing, and diagnostic shell tools (internal)

---

## 🔧 Build + Flash Instructions

1. **Activate virtual environment**
   ```bash
   cd ~/ultimesh
   source .platformio-env/bin/activate
   ```

2. **Build and upload firmware to Heltec ESP32**
   ```bash
   platformio run --target upload
   ```

3. **Monitor serial output**
   ```bash
   platformio device monitor --baud 115200
   ```

4. **Optional: Flash SPIFFS image (for token files etc)**
   ```bash
   platformio run --target uploadfs
   ```

---

## 💻 Shell Modes

ULTIMESH uses persistent shell modes, switched with a leading character:

| Prefix | Mode           | Example                      |
|--------|----------------|------------------------------|
| `:`    | Shell          | `: help`, `: top`            |
| `>`    | Chat/DM        | `> dm node1 hello world`     |
| `/`    | Web nav (WIP)  | `/ go home`                  |
| `~`    | BBS nav (WIP)  | `~ inbox`                    |

---

## 🛠️ Shell Commands (in `:` mode)

- `: list` — List SPIFFS files
- `: free` — Show RAM + Flash usage
- `: cat <file>` — View file contents
- `: rm <file>` — Delete a file
- `: tokens` — Show token table (TODO)
- `: top` — Toggle OLED system monitor
- `: help` — Display this help

---

## 🧾 Revision History

### 🔖 Revision 0.A2
- Shell modes implemented with live prefix switching
- Base commands: `list`, `free`, `cat`, `rm`, `top`, `help`
- Pager screen shown by default; SPIFFS stable

### 🔖 Revision 0.A3
- OLED dual-mode system fully working
- `top` screen added (CPU/temp still `N/A`)
- Time/date shown in pager mode

### 🔖 Revision 0.A4
- Major bugfix: removed duplicate OLED function definitions
- Top mode is now latched (toggles on/off)
- `FL:` display now shows free space only to prevent text overflow
- Repo stabilized and pushed to GitHub

---

## 🧰 Tools Summary

| Script / File        | Purpose                                 |
|----------------------|-----------------------------------------|
| `partitions.csv`     | Custom ESP32 partition layout (6MB+ SPIFFS) |
| `platformio.ini`     | PlatformIO board config                 |
| `.platformio-env/`   | Virtualenv for isolated builds          |
| `/tokens/*.txt`      | Token maps used for encoding LoRa messages |
| `esptool.py` (opt)   | Flash bootloader + firmware + SPIFFS in one step |

---

## 📡 Roadmap

- [ ] Implement `> dm <node> <msg>` with tokenized LoRa sending
- [ ] Add token map editing shell commands
- [ ] Parse incoming LoRa messages and display
- [ ] Fix CPU/TEMP readings on OLED
- [ ] Web/BBS browsing from SPIFFS

---

## 📁 Filesystem Notes

SPIFFS is mounted as root `/`, and file I/O supports listing, deletion, and viewing. Upload `tokens.txt` manually or with `uploadfs` to enable compression.

---

## 🧠 Project Philosophy

> “Everything old is new again.”  
ULTIMESH is about keeping offline tech alive, usable, and portable — even when the world goes dark. ASCII-first. Retro-compatible. Mesh-ready.

---

© 2025 MrTomkinson / oldtechlife.com

---

## 🛠️ Custom Tool Scripts (located in `tools/`)

These scripts bypass PlatformIO's upload behavior to ensure consistent flashing, especially for Heltec ESP32 boards with 8MB flash and large SPIFFS:

| Script              | Purpose                                                    |
|---------------------|------------------------------------------------------------|
| `flash_all.sh`      | Flash firmware + filesystem + bootloader all in one pass  |
| `burn_all.sh`       | Fully erase and reflash everything (factory reset)         |
| `backup_all.sh`     | Backup entire flash image (firmware + SPIFFS)              |
| `backup_firmware.sh`| Save just the firmware portion of flash                    |
| `backup_spiffs.sh`  | Save only the SPIFFS filesystem                            |
| `restore_spiffs.sh` | Restore SPIFFS content from backup                         |

These should be run from the `tools/` directory. Most use `esptool.py` internally and expect:
- A connected ESP32 via `/dev/ttyUSB0` or `/dev/ttyUSB1`
- Pre-built `.bin` files from PlatformIO output folder
