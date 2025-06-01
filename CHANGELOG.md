# ULTIMESH Changelog

## [0.B1] - 2025-06-02

### Major
- New OLED message handling: latched display with return-to-TOP mode
- `/dm` LoRa command support with target filtering
- Config-driven OLED display timing and LoRa behavior

### Added
- Sender ID included in LoRa display for both DM and broadcast
- Shell commands:
  - `> /dm <nodeID> <message>` — direct LoRa messaging
  - `: pager` / `: top` — switch OLED mode
- Extended config.ini parameters:
  - `oled_lora_display_ms`
  - `node_id`
  - `tx_power`, `spread_factor`, `frequency`, etc.

### Improved
- Editor behavior:
  - Supports Backspace and DEL (line removal)
  - Supports long lines with visual truncation
  - Shows modified flag
- LoRa packet validation: non-printable packets ignored
- Debug logging (enable/disable manually)

### Fixed
- OLED would not update after screen rework
- Pager/Top flicker removed
- LoRa messages previously misrouted due to improper checks

### Known Issues
- Line wrapping in OLED is static (not dynamic)
- No ping/whois implementation yet
- Editor is not insert-mode aware



## 🔖 v0.A5 — Shell Core Finalized, Editor Stable

**New Features**
- Editable `/config.ini` loaded at boot
- Shell command `config` to display runtime values
- Tab-completion support for commands and file names
- DOS-style command aliases: `dir`, `copy`, `cls`, etc.
- Minimal file editor with Save, Save As, New, Quit, Help
- Common config handler (`common_config.h/cpp`) for globals

**Fixes**
- Properly displays flat paths in `ls`, `cat`, etc.
- Handles flat SPIFFS correctly (no real directories)
- Prevents invalid file path assumptions (prefix bugs)
- Cleaner shell prompt and newline handling
- Editor cursor & refresh tweaks

**Internal**
- Fully refactored `serial_shell.cpp` with tokenized parsing
- All command logic flattened and modular
- Codebase cleanup for SPIFFS-safe path resolution
- Error-safe token parsing (echo, cp, mv, etc.)

---

## 🔖 v0.A4

- OLED dual-mode system stable
- Added top screen (CPU/temp placeholders)
- Pager mode shows time/date
- SPIFFS layout finalized and GitHub-ready

---

## 🔖 v0.A3

- System monitor mode added with OLED toggle
- Improved serial shell reliability
- Command prefix system (`:`, `>`, `/`, `~`) functional

---

## 🔖 v0.A2

- Shell mode implemented
- Commands: list, free, cat, rm, help
- Initial SPIFFS support (token table planned)

---
## v0.A1 - "Ride the Wave!" (2025-04-26)
- Initial working baseline
- OLED Display splash enabled
- SPIFFS file storage operational
- Serial shell partially working
- Backup and flash scripts started

*More versions and firmware binaries at*: [github.com/mrtomkinson/ultimesh]
