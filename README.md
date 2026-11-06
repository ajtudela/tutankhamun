# Tutankhamun

Tutankhamun is an open-source re-implementation of **Pharaoh + Cleopatra 2.1** (Impressions Games, 2000) in C: the
game written again as a program of its own, `tutankhamun.exe`, that plays on the files of your own copy of the game
in a window (SDL2) on modern Windows. It is also a readable reference for the game's behaviour, mainly to support
[Akhenaten](https://github.com/dalerank/Akhenaten).

Tutankhamun will not run without the original Pharaoh + Cleopatra files. You can buy a digital copy from
[GOG](https://www.gog.com/game/pharaoh_cleopatra) or
[Steam](https://store.steampowered.com/app/564530/Pharaoh__Cleopatra/), or use an original CD-ROM version updated to
2.1.

The goal of the project is to have exactly the same game logic as Pharaoh + Cleopatra, with the same look and feel.
This means that the saved games are compatible with the original game, and any gameplay bugs present in the original
are also present in Tutankhamun. No game assets are part of this repository.

## Running the game

You need an installed Pharaoh + Cleopatra, version 2.1 (Pharaoh with the Cleopatra expansion). Only the game's data
is used, not its executables or DLLs. The game folder is `tutankhamun.exe` with its `config` folder beside it:

    tutankhamun.exe
    config\*.json          the gameplay tables, read at every start
    tutankhamun.ini       written by the game: the game directory and the display settings

Start it with

    tutankhamun.exe [game dir] [save.sav]

Without a game directory it finds your installation itself, taking the first folder that holds the game's data
(`Data\`, `campaign.txt`, `Pharaoh_Text.eng`, `Pharaoh_MM.eng`):

1. the directory remembered in `tutankhamun.ini` beside the program;
2. the current folder, then the folder of tutankhamun.exe;
3. the Steam libraries (`steamapps\common\Pharaoh + Cleopatra`);
4. the GOG installations listed in the registry;
5. a folder picker, asked again until the chosen folder holds the game.

A folder found by 3-5 is remembered in `tutankhamun.ini`. When nothing is found the game says which folders it
tried and what each one lacks. A game directory given on the command line is the only one tried.

Without its `config` folder the game stops with "no configs"; a single file missing from it keeps its built-in
table. Editing the files changes the rules of the game.

Like the original, the game writes into the game directory (the player's records, `status.txt`, `highscore.jas`,
the saved games in `Save\<player>\`), so you need write permission there; play in a copy of the folder to keep your
installation as it is. Saves are in the original format; `.svx` files from Akhenaten are not readable.

**Free play**: the main menu's second button lists every Pharaoh and Cleopatra mission. The chosen one starts with
its briefing as in the campaign, and its end returns to the main menu without changing the player's campaign
progress, mission records or high scores.

**Resolutions**: any size from 640x480 up to the display's (the original's layouts at 640x480, 800x600 and
1024x768, widened layouts for the others). Options > Display settings lists the display's modes, F7 / F8 / F9 and F6
(full screen) work as in the original, and the choice is remembered in `tutankhamun.ini`.

**Discord**: while Discord runs on the same computer, the game shows the mission's title and date in your Discord
status. `config\discord.json` turns it off with `"enabled": 0`.

## Building from source

Windows 10/11 with Visual Studio 2022 ("Desktop development with C++"), CMake 3.22 or newer and git. The game is a
32-bit program. In short:

    build.bat

builds the Release preset and writes the game folder (tutankhamun.exe and config\) into
`out\install\win-msvc-release-vs2022`. [BUILD.md](BUILD.md) has the details and the other ways to build.

Linux and macOS are not supported yet: `update-workspace-linux.sh` and `update-workspace-mac.sh` only say so.

## Command line

    tutankhamun [game dir] [save.sav] [--size WxH] [--window | --fullscreen] [--cursor-scale N] [--log file]
                 [--discord-log]
    tutankhamun --export-configs <dir> | --check-configs <dir>
    tutankhamun --find-game-dir --system

| option | argument | meaning | default |
|---|---|---|---|
| (positional) | game dir | the game directory (a path to a file in it, such as its `.exe`, stands for the folder) | found as above |
| (positional) | save.sav | a saved game loaded after the intro video instead of the main menu | the main menu |
| `--size`, `--resolution` | WxH | the window size for this run, e.g. `1920x1080` | as remembered |
| `--window`, `--windowed` | | start in a window | as remembered |
| `--fullscreen` | | start in full screen | as remembered |
| `--cursor-scale` | N | the size of the mouse cursors: `1`, `1.5` or `2` | `1` |
| `--log` | file | the log file | `tutankhamun_play.log` in the current folder |
| `--discord-log` | | a detailed log of the Discord status in the log file | off |
| `--export-configs` | dir | writes the built-in gameplay tables into dir as JSON (a fresh `config` folder) | |
| `--check-configs` | dir | loads every config of dir and compares it with the built-in tables; exit 0 when all are identical | |
| `--find-game-dir --system` | | shows which game directory the lookup would choose, without starting the game | |

Exit codes: 0 done; 1 a usage error, or a config differs; 2 cannot start (no `config` folder, no game data, the save
cannot be loaded); 3 a fault in the game, written to the log.

## Contributing

This project is not meant for further development: it exists only to preserve the heritage of the original game and
to port it to other platforms. Pull requests are accepted only for minimal fixes. See
[CONTRIBUTING.md](CONTRIBUTING.md).

## Support

- **Issues**: report bugs on [GitHub Issues](https://github.com/dalerank/tutankhamun/issues)
- **Discord**: [discord.gg/HS4njmBvpb](https://discord.gg/HS4njmBvpb)
- **Wiki**: [dalerank.github.io/Akhenaten](https://dalerank.github.io/Akhenaten/)
- **GitHub**: [github.com/dalerank/tutankhamun](https://github.com/dalerank/tutankhamun)
- **Website**: [dalerank.github.io/en](https://dalerank.github.io/en/)

## Credits

Parts of the code come from other projects (details: [LICENSES/README.md](LICENSES/README.md) and each file's
header):

- [Akhenaten](https://github.com/dalerank/Akhenaten): the Bink video decoder (src/third_party/akhenaten_bink), the SDL
  build scripts (cmake/), the Discord presence, the mouse cursors, the Discord and Patreon pictures of the main menu's
  link buttons, the model of the resolution list and of the link buttons;
- [Julius](https://github.com/bvschaik/julius): src/platform/sound_device.c and src/platform/screen.c, the cursors'
  pixel art, and the select list that the resolution list draws like (src/game/window/display_resolution_list.c);
- the GitHub picture of the link buttons is the Simple Icons mark (CC0-1.0).

Third-party libraries: SDL2 2.32.10 (zlib), SDL2_mixer 2.8.0 (zlib) with minimp3 (CC0-1.0), parson 1.5.3 (MIT,
src/third_party/parson); their licence texts are in LICENSES/.

## License

The project's own code is under the MIT License ([LICENSE](LICENSE)); the files taken from Julius and Akhenaten keep
their AGPL-3.0 ([LICENSES/AGPL-3.0.txt](LICENSES/AGPL-3.0.txt), listed in LICENSES/README.md with the libraries'
licences). A program built with those files, as every build of tutankhamun is, is distributed under AGPL-3.0 as a
whole.

Tutankhamun is not affiliated with or endorsed by the owners of Pharaoh and Cleopatra. The game's data, names and
trademarks belong to their owners; you need your own copy of the game to play.
