#!/usr/bin/env bash
# Builds the iPhone and iPad app Tutankhamun.app as Julius's CI builds its iOS app (.ci_scripts/run_cmake.sh and
# run_build.sh, "ios"): CMake with the Xcode generator for iOS (static SDL2 and SDL2_mixer for arm64 iphoneos,
# cmake/TutankhamunBuildSDL.cmake), then xcodebuild. Without a team the app is not signed: a check of the build, as in
# Julius's CI, and Tutankhamun.ipa that a signing tool can install later. With TUTANKHAMUN_IOS_TEAM=<team ID> (Xcode
# signed in to the developer account) Xcode signs it for that team's devices with automatic signing.
#
#   packaging/ios/build_ios.sh [build dir]
#
# The build dir defaults to build/ios. Needs Xcode, CMake 3.22+ and git (brew install cmake). The result is
# Tutankhamun.ipa in the build dir.
set -euo pipefail

source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="$(mkdir -p "${1:-$source_dir/build/ios}" && cd "${1:-$source_dir/build/ios}" && pwd)"
team="${TUTANKHAMUN_IOS_TEAM:-}"

cmake -S "$source_dir" -B "$build_dir" -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DTUTANKHAMUN_IOS_TEAM="$team"

if [ -n "$team" ]; then
    signing=(-allowProvisioningUpdates)
else
    signing=(CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO CODE_SIGN_IDENTITY="")
fi
xcodebuild -project "$build_dir/tutankhamun.xcodeproj" -target tutankhamun -configuration Release -sdk iphoneos \
    "${signing[@]}" build

app="$(find "$build_dir/bin" -maxdepth 2 -name Tutankhamun.app -type d | head -n 1)"
if [ -z "$app" ]; then
    echo "Tutankhamun.app was not built" >&2
    exit 1
fi
echo "the app: $app"
ls "$app"

ipa="$build_dir/Tutankhamun.ipa"
stage="$build_dir/ipa"
rm -rf "$stage" "$ipa"
mkdir -p "$stage/Payload"
cp -R "$app" "$stage/Payload/"
(cd "$stage" && zip -qry "$ipa" Payload)
rm -rf "$stage"
echo "app archive: $ipa"
