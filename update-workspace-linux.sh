#!/usr/bin/env sh
# Configures the Linux workspace (64-bit, static SDL2 + SDL2_mixer + Bink, Ninja) through CMakePresets.json:
# out/build/linux-dev or out/build/linux-release, with compile_commands.json for the editor.
# Usage: update-workspace-linux.sh [Debug|Release]  (default: Debug). The AppImage: packaging/linux/build_appimage.sh.
set -e
case "${1:-Debug}" in
    Debug | debug) preset=linux-dev ;;
    Release | release) preset=linux-release ;;
    *)
        echo "usage: update-workspace-linux.sh [Debug|Release]" >&2
        exit 2
        ;;
esac
cd "$(dirname "$0")"
cmake --preset "$preset"
echo "update-workspace-linux.sh: configured out/build/$preset; build it with: cmake --build --preset $preset"
