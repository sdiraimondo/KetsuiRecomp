# Ketsui (Xbox 360) - game identity

Source: `/run/media/stephane/BC250_USB/ROMs/X360/Ketsui` (USB drive, see note below)
Exposed in the repo as `game_files/` (symlink, not a copy).

| Field | Value |
| --- | --- |
| Title ID | `0x465607D1` (from XEX execution info) |
| Media ID | `0x36631DF6` (execution info; the 16-byte media ID in security info ends with the same value) |
| Version / base version | `0x00000001` / `0x00000001` |
| Disc | 1 of 1, platform 0, executable table 0 |
| SHA-1 of default.xex | `2e7a1006084f50798ba51513c68768e7ba06abbb` |
| File size | 15,646,720 bytes (`0xEEC000`) |
| XEX2 module flags | `0x00000001` |
| Image base (load address) | `0x82000000` |
| Entry point | `0x828D2438` |
| Security header size / image size | `0x1EF4` / `0x13A0000` |
| Game regions (raw, security info) | `0x00000004` (mapping to a named region not verified; the SDK does not decode it) |
| Allowed media types | `0x0000013A` |
| Optional headers | 14 (decoded with `/tmp/opencode/xexinfo.py`) |

Notes:
- The path given as `GAME_FILES` (`/home/stephane/Téléchargements/Ketsui`) does not exist. The same layout (default.xex, Data/, nxeart, $SystemUpdate/) is on the USB drive, so `game_files` points there.
- The game is never copied into the repo.
