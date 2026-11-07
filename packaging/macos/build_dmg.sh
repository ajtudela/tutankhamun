#!/usr/bin/env bash
# Builds the macOS disk image of tutankhamun as Akhenaten builds its macOS app (.github/workflows/akhenaten_mac_*.yml,
# compile_macos.sh): one Release build per architecture with Unix Makefiles (static SDL2 + SDL2_mixer for that
# architecture, cmake/TutankhamunBuildSDL.cmake), the oldest macOS 11.7 for arm64 and 10.15 for x86_64; "universal"
# builds both and joins the two programs with lipo. The app bundle Tutankhamun.app is signed ad hoc (no developer
# certificate: on first start the user opens it with right click, Open) and put into a compressed disk image beside a
# link to /Applications.
#
#   packaging/macos/build_dmg.sh [arm64|x86_64|universal] [build dir]
#
# The architecture defaults to this Mac's, the build dir to build/macos. Needs the Xcode command line tools, CMake
# 3.22+ and git (brew install cmake). The result is Tutankhamun-<architecture>.dmg in the build dir; the single
# architecture builds are in <build dir>/<architecture>.
set -euo pipefail

source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
arch="${1:-$(uname -m)}"
build_dir="$(mkdir -p "${2:-$source_dir/build/macos}" && cd "${2:-$source_dir/build/macos}" && pwd)"
cores="$(sysctl -n hw.ncpu)"

build_arch() {
    local one="$1" target
    case "$one" in
        arm64) target=11.7 ;;
        x86_64) target=10.15 ;;
    esac
    cmake -S "$source_dir" -B "$build_dir/$one" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$one" -DCMAKE_OSX_DEPLOYMENT_TARGET="$target"
    cmake --build "$build_dir/$one" -- -j"$cores"
}

case "$arch" in
    arm64 | x86_64)
        build_arch "$arch"
        app="$build_dir/$arch/bin/Tutankhamun.app"
        ;;
    universal)
        build_arch arm64
        build_arch x86_64
        app="$build_dir/universal/Tutankhamun.app"
        rm -rf "$app"
        mkdir -p "$build_dir/universal"
        cp -R "$build_dir/arm64/bin/Tutankhamun.app" "$app"
        lipo -create "$build_dir/arm64/bin/Tutankhamun.app/Contents/MacOS/Tutankhamun" \
            "$build_dir/x86_64/bin/Tutankhamun.app/Contents/MacOS/Tutankhamun" \
            -output "$app/Contents/MacOS/Tutankhamun"
        # the arm64 bundle's Info.plist says 11.7; the joined program runs from 10.15 on Intel
        plutil -replace LSMinimumSystemVersion -string 10.15 "$app/Contents/Info.plist"
        ;;
    *)
        echo "usage: build_dmg.sh [arm64|x86_64|universal] [build dir]" >&2
        exit 2
        ;;
esac

echo "the program's architectures: $(lipo -archs "$app/Contents/MacOS/Tutankhamun")"
codesign --force --deep --sign - "$app"
codesign --verify --verbose "$app"

dmg="$build_dir/Tutankhamun-$arch.dmg"
stage="$build_dir/dmg-$arch"
rm -rf "$stage" "$dmg"
mkdir -p "$stage"
cp -R "$app" "$stage/"
ln -s /Applications "$stage/Applications"
hdiutil create -volname Tutankhamun -srcfolder "$stage" -ov -format UDZO "$dmg"
rm -rf "$stage"
echo "disk image: $dmg"
