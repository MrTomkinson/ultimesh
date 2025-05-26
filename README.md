# ULTIMESH
Retro-styled portable LoRa mesh firmware for modern and vintage devices.

 ____  _ _ _                         _
|  _ \(_) | |_   _ _ __ ___  __ _  __| |___
| |_) | | | | | | | '__/ _ \/ _` |/ _` / __|
|  _ <| | | | |_| | | |  __/ (_| | (_| \__ \
|_| \_\_|_|_|\__, |_|  \___|\__,_|\__,_|___/
             |___/
 __  __           _     _     _
|  \/  | ___   __| | __| | __| |___
| |\/| |/ _ \ / _` |/ _` |/ _` / __|
| |  | | (_) | (_| | (_| | (_| \__ \
|_|  |_|\___/ \__,_|\__,_|\__,_|___/

+-----------------------------------------+
| 🌊 v0.A1 - "Ride The Wave!"              |
|                                         |
| [✔] PlatformIO Boot + SPIFFS            |
| [✔] OLED Splash + Drive Stats           |
| [✔] Basic Shell (list, free, cat, rm)   |
| [✔] Token Encoder / Decoder             |
| [✔] GitHub Source Control Live          |
| [✔] Full Backup / Flash Scripts         |
| [~] Expanded Shell Commands (working)   |
| [ ] First Live LoRa Send/Receive         |
| [ ] CF Card Parallel Integration        |
| [ ] Simple Web Server                   |
| [ ] IRC Gateway (future)                |
| [ ] BBS Emulation (future)              |
+-----------------------------------------+



- Project: ULTIMESH
- Version: v0.A1 - "Ride the Wave!"
- Status: Alpha (working prototype)
- License: MIT
- Author: MrTomkinson + Project Contributors


# ULTIMESH Firmware - Project Status

## Current Firmware Version
- **v0.A1 - "Ride the wave!"**

## Changelog

### v0.A1 - "Ride the wave!" (April 26, 2025)
- ✅ Minimal core boot verified
- ✅ SPIFFS filesystem successfully mounted
- ✅ Token table loaded dynamically (`/tokens.txt`)
- ✅ LoRa radio initialized successfully
- ✅ Basic OLED display setup reintroduced
- ✅ ULTIMESH Splash screen added at boot
- 🚧 Ongoing: Unified Shell refinements and Parallel/Serial bridging

---

## Notes
- Current builds are considered **experimental ALPHA quality** ("Hold onto your boardshorts!")
- Full unit backups are recommended before flashing (see `/tools/` scripts)
- Compatible with Heltec WiFi LoRa 32 (V2)
