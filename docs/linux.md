# Tutankhamun on Linux

[← README](../README.md)

## Running

[linux_appimage.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_linux/main/linux_appimage.zip)
holds `Tutankhamun-x86_64.AppImage`, one file with the program and its configs. It runs on 64-bit distributions with
glibc 2.34 or newer (Ubuntu 22.04, Debian 12, Fedora 35 and later) under X11 or Wayland, with PulseAudio, PipeWire or
ALSA sound:

    chmod +x Tutankhamun-x86_64.AppImage
    ./Tutankhamun-x86_64.AppImage [game dir] [save.sav]

(all options: [the command line](command-line.md)). You need the game's data from an installed Pharaoh + Cleopatra,
for example the Steam version installed through Proton or the GOG version installed with Wine or Heroic; the data
files can also just be copied from a Windows installation. The game directory is looked for as
[on Windows](windows.md#running), with these differences:

- Steam: the `steamapps` of `~/.steam/steam`, `~/.local/share/Steam`, the Flatpak and the Snap Steam, and the
  libraries listed in their `libraryfolders.vdf`;
- GOG: `~/GOG Games`, `~/Games/Heroic` and the Wine prefix `~/.wine` (Pharaoh + Cleopatra or Pharaoh Gold);
- the folder picker is `zenity` or `kdialog` when one of them is installed.

The file names of the game's data may be in any case (`Data` or `DATA`, `Audio` or `AUDIO`). `tutankhamun.ini` and
the log `tutankhamun_play.log` are in `~/.config/tutankhamun/` (`$XDG_CONFIG_HOME/tutankhamun/`); a folder
`~/.config/tutankhamun/config` with your own gameplay tables replaces the configs inside the AppImage
(`./Tutankhamun-x86_64.AppImage --export-configs ~/.config/tutankhamun/config` writes a full set to edit). The game
writes the players and saved games into the game directory, as on Windows.

## Building

A 64-bit Linux with GCC 9+ or Clang 10+, CMake 3.22 or newer, Ninja, git, wget and the development headers SDL is
built against (SDL loads X11, Wayland, ALSA and PulseAudio when the game starts, they are not linked). On Debian and
Ubuntu:

    sudo apt install build-essential cmake ninja-build git wget libgl1-mesa-dev libx11-dev libxext-dev libxrandr-dev \
        libxcursor-dev libxi-dev libxss-dev libxfixes-dev libwayland-dev libxkbcommon-dev wayland-protocols \
        libdecor-0-dev libasound2-dev libpulse-dev libdbus-1-dev libudev-dev

| command | builds | result |
|---|---|---|
| `./update-workspace-linux.sh [Debug\|Release]`, then `cmake --build --preset linux-dev` (or `linux-release`) | the `linux-dev` (Debug, the default) or `linux-release` preset, with `compile_commands.json` | `out/build/<preset>/bin/tutankhamun` with `config/` beside it |
| `packaging/linux/build_appimage.sh [build dir]` | Release, installed into an AppDir and packed by linuxdeploy | `build/appimage/Tutankhamun-x86_64.AppImage` (or in the given folder) |

The first configure fetches SDL2 2.32.10 and SDL2_mixer 2.8.0 and builds them as static libraries, as on Windows;
the C++ run time is linked in too, so the program needs only the C library. Run the preset build as
`out/build/linux-release/bin/tutankhamun [game dir] [save.sav]`. build_appimage.sh downloads linuxdeploy on the first
run (`LINUXDEPLOY=<path>` uses one you have). An AppImage needs a glibc at least as new as the one it was built with,
so build it on the oldest distribution it should run on: the CI builds on Ubuntu 22.04 (glibc 2.34).

[BUILD.md](../BUILD.md#linux) has the details.
