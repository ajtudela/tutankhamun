#!/usr/bin/env bash
# Builds the macOS disk image of tutankhamun, after Julius (.ci_scripts/run_cmake.sh, run_build.sh): a Release build
# of the app bundle Tutankhamun.app (static SDL2 + SDL2_mixer, cmake/TutankhamunBuildSDL.cmake; arm64 + x86_64 in one
# universal binary), signed ad hoc (no developer certificate: on first start the user opens it with right click, Open)
# and put into a compressed disk image beside a link to /Applications.
#
#   packaging/macos/build_dmg.sh [build dir]       (default: build/macos)
#
# Needs the Xcode command line tools, CMake 3.22+, git and Ninja (brew install cmake ninja). ARCHS=<list> builds other
# architectures than "arm64;x86_64" (ARCHS=arm64 for this Mac only, faster). The result is Tutankhamun.dmg in the
# build dir.
set -euo pipefail

source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="$(mkdir -p "${1:-$source_dir/build/macos}" && cd "${1:-$source_dir/build/macos}" && pwd)"
archs="${ARCHS:-arm64;x86_64}"
generator=()
if command -v ninja >/dev/null 2>&1; then
    generator=(-G Ninja)
fi

# (the macOS bash is 3.2: an empty array is expanded through ${a[@]+...} under set -u)
cmake -S "$source_dir" -B "$build_dir" ${generator[@]+"${generator[@]}"} -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=$archs"
cmake --build "$build_dir" --parallel

app="$build_dir/bin/Tutankhamun.app"
echo "the binary's architectures: $(lipo -archs "$app/Contents/MacOS/Tutankhamun")"
codesign --force --deep --sign - "$app"
codesign --verify --verbose "$app"

stage="$build_dir/dmg"
rm -rf "$stage" "$build_dir/Tutankhamun.dmg"
mkdir -p "$stage"
cp -R "$app" "$stage/"
ln -s /Applications "$stage/Applications"
hdiutil create -volname Tutankhamun -srcfolder "$stage" -ov -format UDZO "$build_dir/Tutankhamun.dmg"
rm -rf "$stage"
echo "disk image: $build_dir/Tutankhamun.dmg"
