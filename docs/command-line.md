# The command line

[← README](../README.md)

The desktop programs (Windows, Linux, macOS) take the same command line:

    tutankhamun [game dir] [save.sav] [--size WxH] [--window | --fullscreen] [--cursor-scale N] [--log file]
                 [--discord-log] [--no-update]
    tutankhamun --export-configs <dir> | --check-configs <dir>
    tutankhamun --find-game-dir --system

| option | argument | meaning | default |
|---|---|---|---|
| (positional) | game dir | the game directory (a path to a file in it, such as its `.exe`, stands for the folder) | found as described on [Windows](windows.md#running), [Linux](linux.md#running), [macOS](macos.md#running) |
| (positional) | save.sav | a saved game loaded after the intro video instead of the main menu | the main menu |
| `--size`, `--resolution` | WxH | the window size for this run, e.g. `1920x1080` | as remembered |
| `--window`, `--windowed` | | start in a window | as remembered |
| `--fullscreen` | | start in full screen | as remembered |
| `--cursor-scale` | N | the size of the mouse cursors: `1`, `1.5` or `2` | `1` |
| `--log` | file | the log file | `tutankhamun_play.log` in the current folder (on Linux in `~/.config/tutankhamun`, on macOS in `~/Library/Application Support/tutankhamun`) |
| `--discord-log` | | a detailed log of the Discord status in the log file | off |
| `--no-update` | | no check for a newer build at this start ([Windows](windows.md#running), "Updates") | the check as `tutankhamun.ini` says |
| `--export-configs` | dir | writes the built-in gameplay tables into dir as JSON (a fresh `config` folder) | |
| `--check-configs` | dir | loads every config of dir and compares it with the built-in tables; exit 0 when all are identical | |
| `--find-game-dir --system` | | shows which game directory the lookup would choose, without starting the game | |

Exit codes: 0 done; 1 a usage error, or a config differs; 2 cannot start (no `config` folder, no game data, the save
cannot be loaded); 3 a fault in the game, written to the log.
