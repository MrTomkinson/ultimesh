# ULTIMESH TODO / DEV NOTES

## Shell Feature Roadmap

| Feature          | Status     | Notes                          |
|------------------|------------|--------------------------------|
| ls               | ✅ Flat mode | SPIFFS: show all files        |
| cat/edit/rm      | ✅ Works     | Full paths accepted           |
| cd, mkdir, pwd   | ⚠️ Disabled | Warn in SPIFFS mode           |
| tokens cmd       | ✅ Active    | Shows current token map       |
| OLED `:top`      | ✅ Toggleable | Pager/system info             |
| clear / cls      | ✅ Works     | ANSI clear screen             |
| help             | ✅ Works     | Lists available commands      |
| LoRa DM `> dm`   | 🚧 Planned   | Stubbed, to be implemented    |
| web/BBS nav      | 🚧 Planned   | : / ~ mode placeholders       |

---

## Terminal UX Enhancements (Upcoming)

| Feature             | Needed? | Notes                         |
|---------------------|---------|-------------------------------|
| Tab Completion      | ✅      | For file/command suggestions  |
| Command History ↑↓  | ✅      | Arrow keys navigate history   |
| Configurable Keys   | ⚙️      | Allow user remaps (later)     |
| Text-based Menus    | ⬛      | For editor, help, etc         |

---

## Editor Keybindings

| Key Combo       | Action        |
|------------------|---------------|
| Ctrl+S           | Save          |
| Ctrl+A           | Save As       |
| Ctrl+N           | New file      |
| Ctrl+Q           | Quit editor   |
| Ctrl+I           | Help/info     |
| Arrow Keys       | Cursor move   |
| Enter            | Insert line   |
| Backspace        | Delete char   |

---

## Planned Docs

| Topic            | Format              | Location          |
|------------------|---------------------|-------------------|
| Shell help       | `: help` and `: help <cmd>` | CLI |
| Editor shortcuts | Shown with Ctrl+I   | In-editor overlay |
| Full manual      | `/help/index.html`  | SPIFFS or CF card |

---

## Storage Notes

- SPIFFS: flat file, path-based naming
- CF card (future): real folder support

