# Tutankhamun on macOS

[← README](../README.md)

## Running

[macos_arm_build.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_macos/main/macos_arm_build.zip)
holds `Tutankhamun-arm64.dmg` for Apple silicon Macs (macOS 11.7 or newer),
[macos_x64_build.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_macos/main/macos_x64_build.zip)
holds `Tutankhamun-x86_64.dmg` for Intel Macs (macOS 10.15 or newer). Open the disk image and drag `Tutankhamun.app`
to Applications. The app is signed ad hoc, not by a registered developer, so macOS refuses it at the first start:
open it with right click (Control-click) > Open > Open, or allow it in System Settings > Privacy & Security, or run
`xattr -cr /Applications/Tutankhamun.app`.

You need the game's data from an installed Pharaoh + Cleopatra, for example copied from a Windows installation or
from the GOG or Steam version installed with CrossOver or Wine. Started from the Finder, the game asks for the folder
with the Finder's folder dialog and remembers it. The game directory is looked for as [on Linux](linux.md#running),
plus the Steam library in `~/Library/Application Support/Steam`. From the Terminal it takes
[the command line](command-line.md):

    /Applications/Tutankhamun.app/Contents/MacOS/Tutankhamun [game dir] [save.sav]

`tutankhamun.ini` and the log `tutankhamun_play.log` are in `~/Library/Application Support/tutankhamun/`; a folder
`config` there with your own gameplay tables replaces the configs inside the app. The game writes the players and
saved games into the game directory, as on Windows.

## Building

The Xcode command line tools (`xcode-select --install`), CMake 3.22 or newer and git, for example from Homebrew
(`brew install cmake`). The build is the one of Akhenaten: Unix Makefiles and one architecture per build, arm64
(macOS 11.7+) or x86_64 (macOS 10.15+).

| command | builds | result |
|---|---|---|
| `./update-workspace-mac.sh [Debug\|Release]`, then `cmake --build --preset macos-dev` (or `macos-release-arm64`, `macos-release-x86_64`) | the `macos-dev` (Debug, the default) or the Release preset of this Mac's architecture, with `compile_commands.json` | `out/build/<preset>/bin/Tutankhamun.app` |
| `packaging/macos/build_dmg.sh [arm64\|x86_64\|universal] [build dir]` | Release for the architecture (default: this Mac's), signed ad hoc and put into a disk image; `universal` builds both and joins them with lipo | `build/macos/Tutankhamun-<architecture>.dmg` (or in the given folder) |

As on the other systems the first configure fetches SDL2 and SDL2_mixer and builds them as static libraries for the
build's architecture, so the app needs nothing but the system. The configs are copied into the app's
`Contents/Resources/config` at every build.
