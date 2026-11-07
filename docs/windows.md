# Tutankhamun on Windows

[← README](../README.md)

## Running

[windows_build.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_windows/main/windows_build.zip)
holds the 32-bit program, [windows_build_x64.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_windows/main/windows_build_x64.zip)
the 64-bit one: they play the same game and read and write the same saved games. Unpack it anywhere:

    tutankhamun.exe
    config\*.json          the gameplay tables, read at every start
    tutankhamun.ini       written by the game: the game directory and the display settings

Start it with

    tutankhamun.exe [game dir] [save.sav]

(all options: [the command line](command-line.md)). You need an installed Pharaoh + Cleopatra, version 2.1 (Pharaoh
with the Cleopatra expansion); only the game's data is used, not its executables or DLLs. Without a game directory
the game finds your installation itself, taking the first folder that holds the game's data (`Data\`,
`campaign.txt`, `Pharaoh_Text.eng`, `Pharaoh_MM.eng`):

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

**Resolutions**: any size from 640x480 up to the display's (the original's layouts at 640x480, 800x600 and
1024x768, widened layouts for the others). Options > Display settings lists the display's modes, F7 / F8 / F9 and F6
(full screen) work as in the original, and the choice is remembered in `tutankhamun.ini`.

**Discord**: while Discord runs on the same computer, the game shows the mission's title and date in your Discord
status. `config\discord.json` turns it off with `"enabled": 0`.

## Building

Windows 10/11 with Visual Studio 2022 ("Desktop development with C++"), CMake 3.22 or newer and git. The game builds
as a 32-bit (Win32, the default) or a 64-bit (x64) program. In short:

    build.bat [release|debug] [vs|ninja] [x86|x64]

| option | values | meaning | default |
|---|---|---|---|
| configuration | `release`, `debug` | the Release or the Debug build | `release` |
| generator | `vs`, `ninja` | Visual Studio 2022, or Ninja (on PATH) in the vcvars32 / vcvars64 environment that build.bat sets up | `vs` |
| architecture | `x86`, `x64` | a 32-bit (Win32) or a 64-bit (x64) program | `x86` |

The options go in this order, so an architecture needs the two before it: `build.bat` builds the 32-bit Release with
Visual Studio, `build.bat release vs x64` the 64-bit one, `build.bat debug ninja x64` the 64-bit Debug with Ninja.
The build is in `out\build\<preset>` and the game folder (tutankhamun.exe and config\, copy both) in
`out\install\<preset>`, the preset being `win-msvc-<release|dev>[-x64]-<vs2022|ninja>`: `build.bat` writes
`out\install\win-msvc-release-vs2022`, `build.bat release vs x64` writes `out\install\win-msvc-release-x64-vs2022`.
`TUTANKHAMUN_JOBS` limits the parallel build jobs. The exit code is 0 when the game folder is complete, 1 when the
build failed and 2 for an unknown option.

[BUILD.md](../BUILD.md) has the details and the other ways to build (the presets, plain CMake).
