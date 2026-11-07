# Building tutankhamun

## What you need

- Windows 10 or 11.
- Visual Studio 2022 with the "Desktop development with C++" workload (MSVC x86 tools, the Windows SDK).
- CMake 3.22 or newer (the one that comes with Visual Studio is enough) and git on PATH: the first configure of a
  build folder fetches SDL2 2.32.10 and SDL2_mixer 2.8.0 from GitHub and builds them as static libraries.
- Ninja, only for the Ninja presets.

The presets build a 32-bit program (Win32) or, with `-x64` in their name, a 64-bit one (x64).

## build.bat

    build.bat [release|debug] [vs|ninja] [x86|x64]

configures and builds one preset (default `release vs x86`; `x64` builds the 64-bit program), installs the game folder (tutankhamun.exe and `config\`)
into `out\install\<preset>` and prints both paths. With `ninja` it sets up the vcvars32 environment itself;
`TUTANKHAMUN_JOBS` limits the parallel jobs. `update-workspace.bat [Debug|Release] [x86|x64]` configures the Visual Studio preset
and opens its solution.

## Presets

| configuration | Visual Studio 2022 (Win32) | Visual Studio 2022 (x64) | Ninja (vcvars32 environment) | Ninja (vcvars64 environment) |
|---|---|---|---|---|
| Debug | `win-msvc-dev-vs2022` | `win-msvc-dev-x64-vs2022` | `win-msvc-dev-ninja` | `win-msvc-dev-x64-ninja` |
| Release | `win-msvc-release-vs2022` | `win-msvc-release-x64-vs2022` | `win-msvc-release-ninja` | `win-msvc-release-x64-ninja` |

    cmake --preset win-msvc-release-vs2022
    cmake --build --preset win-msvc-release-vs2022

## Plain CMake

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Release

The program is `build\bin\Release\tutankhamun.exe` with `config\` beside it (copied on every build).
`cmake --install build --config Release --prefix <folder>` writes only those two. SDL2, SDL2_mixer and the C run time
are linked statically: no DLL has to be copied.

| CMake option | default | effect |
|---|---|---|
| `TUTANKHAMUN_SDL_LIBS_DIR` | `<build folder>\libs` | where the static SDL2 and SDL2_mixer are built and installed; a folder that already holds them is used as it is, so build folders can share one |

Building offline: `-DFETCHCONTENT_SOURCE_DIR_SDL2=<dir>` and `-DFETCHCONTENT_SOURCE_DIR_SDL2_MIXER=<dir>` take SDL
from source folders you already have.

## Checking the configs

`tutankhamun --check-configs config` loads every file of the `config` folder and compares it with the tables built
into the program (exit 0 when all are identical); `tutankhamun --export-configs <dir>` writes a fresh set.
