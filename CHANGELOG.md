# ULTIMESH Changelog

## [v0.5.0] - 2025-06-04
### Added
- ✅ JSON-based command loading via `/commands.json`
- ✅ Tab completion for both commands and file paths
- ✅ Command history (Up/Down arrows) in shell mode
- ✅ SSH mode now fully supports dynamic command execution
- ✅ LoRa DM support with reply tracking (`@r`)

### Changed
- 🔁 `serial_shell.cpp` was fully cleaned up and modernized
- 🔧 Replaced all hardcoded commands with dynamic dispatcher

### Fixed
- 🛠 Stack overflow on large JSON loads (handled with increased stack and scoped doc lifetime)
- 🐞 Multiple definition/linker errors with `firmwareVersion`, `reverseMap`, and `lastDMFrom`

### Notes
- `commands.json` must be valid and small enough to parse in RAM
- Commands now show up in `: help` dynamically

## [0.b2] - 2025-06-05
### Added
- Modular shell modes: `:`, `>`, `@`, `#` for shell, broadcast, DM, and SSH respectively
- `@r` command to reply to the last DM sender
- `serial_shell` modular refactor with individual handlers for DM/SSH/Broadcast

### Fixed
- Improved DM routing and parsing
- Resolved command execution flow for SSH sessions
- Prevented accidental self-DM

### Known Issues
- Occasional broadcast parsing inconsistency (may be due to packet drop)
- Prompt occasionally disappears after SSH/DMS responses


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
