#!/usr/bin/env bash
# Builds the Linux AppImage of tutankhamun: a Release build (static SDL2 + SDL2_mixer, cmake/TutankhamunBuildSDL.cmake),
# installed into an AppDir with the configs, the desktop entry, the icons and the AppStream metadata, then packed by
# linuxdeploy. Build it on the oldest distribution the AppImage should run on (the CI uses Ubuntu 22.04): the
# AppImage needs a glibc at least as new as the build's.
#
#   packaging/linux/build_appimage.sh [build dir]       (default: build/appimage)
#
# Needs CMake 3.22+, a C/C++ compiler, git, wget and the development headers SDL uses (X11, Wayland, ALSA,
# PulseAudio; see .github/workflows/tutankhamun_linux.yml). LINUXDEPLOY=<path> uses a linuxdeploy already downloaded.
# The result is Tutankhamun-x86_64.AppImage in the build dir.
set -euo pipefail

source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="$(mkdir -p "${1:-$source_dir/build/appimage}" && cd "${1:-$source_dir/build/appimage}" && pwd)"
app_dir="$build_dir/AppDir"
arch="$(uname -m)"

cmake -S "$source_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$build_dir" --parallel
rm -rf "$app_dir"
DESTDIR="$app_dir" cmake --install "$build_dir"

linuxdeploy="${LINUXDEPLOY:-$build_dir/linuxdeploy-$arch.AppImage}"
if [ ! -x "$linuxdeploy" ]; then
    wget -q -O "$linuxdeploy" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$arch.AppImage"
    chmod +x "$linuxdeploy"
fi

cd "$build_dir"
export APPIMAGE_EXTRACT_AND_RUN=1
export OUTPUT="Tutankhamun-$arch.AppImage"
"$linuxdeploy" --appdir "$app_dir" \
    --desktop-file "$app_dir/usr/share/applications/tutankhamun.desktop" \
    --icon-file "$app_dir/usr/share/icons/hicolor/256x256/apps/tutankhamun.png" \
    --output appimage
echo "AppImage: $build_dir/$OUTPUT"
