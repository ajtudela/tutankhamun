# Tutankhamun on the iPad and iPhone

[← README](../README.md)

## Running

[ios_build.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_ios/main/ios_build.zip) holds
`Tutankhamun.ipa`, the app for iOS and iPadOS 13 or newer (arm64). The CI's build is not signed, as Julius's iOS build:
it shows that the game builds for iOS. To run it on a device, build it signed for your developer team (below), or sign
the `.ipa` with your own tool.

The app needs the game's data from an installed Pharaoh + Cleopatra 2.1, for example copied from a Windows
installation. In the Files app, under On My iPad > Tutankhamun, put the game folder (the one with `Data`,
`campaign.txt` and `Pharaoh_Text.eng`), or its contents. At start the game looks for it in the app's Documents and in
each folder there. It is played in landscape with touch, as [on Android](android.md): a tap is a left click, a long
press a right click, a drag a left drag, and a tap in a text field opens the on-screen keyboard. The game writes
the players and saved games into the game folder, so they are seen in the Files app too.

`tutankhamun.ini` and the log `tutankhamun_play.log` are in the app's `Library/Application Support/tutankhamun/`; the
configs are inside the app.

## Building

A Mac with Xcode, CMake 3.22 or newer and git (`brew install cmake`). As Julius builds for iOS: the CMake project with
the Xcode generator and `CMAKE_SYSTEM_NAME=iOS`, SDL2 and SDL2_mixer built as static libraries for arm64 iphoneos at
the first configure, then `xcodebuild`.

| command | builds | result |
|---|---|---|
| `packaging/ios/build_ios.sh [build dir]` | Release, not signed | `build/ios/Tutankhamun.ipa` (or in the given folder) |
| `TUTANKHAMUN_IOS_TEAM=<team ID> packaging/ios/build_ios.sh` | Release, signed by Xcode for the team's devices (automatic signing; Xcode signed in to the developer account) | the same `.ipa`, to install on a registered device |
| `cmake -S . -B build/ios-xcode -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64 -DTUTANKHAMUN_IOS_TEAM=<team ID>`, then open `build/ios-xcode/tutankhamun.xcodeproj` | the Xcode project | Run on a connected iPad from Xcode |

The team ID is in the developer account's Membership details. The bundle identifier is
`com.github.dalerank.tutankhamun`; the configs are copied into the app at every build.
