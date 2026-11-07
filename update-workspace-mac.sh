#!/usr/bin/env sh
# Configures the macOS workspace (static SDL2 + SDL2_mixer + Bink, Ninja) through CMakePresets.json:
# out/build/macos-dev (this Mac's architecture) or out/build/macos-release (universal arm64 + x86_64), with
# compile_commands.json for the editor; the program is the app bundle bin/Tutankhamun.app.
# Usage: update-workspace-mac.sh [Debug|Release]  (default: Debug). The disk image: packaging/macos/build_dmg.sh.
set -e
case "${1:-Debug}" in
    Debug | debug) preset=macos-dev ;;
    Release | release) preset=macos-release ;;
    *)
        echo "usage: update-workspace-mac.sh [Debug|Release]" >&2
        exit 2
        ;;
esac
cd "$(dirname "$0")"
cmake --preset "$preset"
echo "update-workspace-mac.sh: configured out/build/$preset; build it with: cmake --build --preset $preset"
