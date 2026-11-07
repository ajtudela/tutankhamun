# Tutankhamun on Android

[← README](../README.md)

## Running

[android_apk.zip](https://nightly.link/dalerank/tutankhamun/workflows/tutankhamun_android/main/android_apk.zip)
holds `app-release.apk`; it runs on Android 5.0 or newer (arm64-v8a, armeabi-v7a and x86_64). Copy your whole
Pharaoh + Cleopatra folder (the one with `Data`, `campaign.txt` and `Pharaoh_Text.eng`) to the device, for example
into `Download`; at the first start the game asks for that folder and remembers it. It reads the data and writes the
players and saved games there, as on Windows.

The game is full screen in landscape. A tap is a left click, a long press a right click, a drag a left drag, and
the back button is Escape. A tap in a text field opens the on-screen keyboard. With a building chosen, the first tap
on the map shows where it goes and a second tap on it builds it; roads, walls, canals, housing and the other tools
that are dragged are dragged as with a mouse; the button at the top right of the map (or a long press) drops the
building. The release APK is signed with a debug key: uninstall an APK signed with another key first.

## Building

JDK 17 and the Android SDK with platform 36, NDK 27.2.12479018 and CMake 3.22.1. Android Studio installs them
(SDK Manager), or with the command-line tools:

    sdkmanager "platforms;android-36" "build-tools;36.0.0" "ndk;27.2.12479018" "cmake;3.22.1"

Gradle finds the SDK through `ANDROID_HOME` or `android/local.properties` (`sdk.dir=/path/to/Android/Sdk`; on Windows
`sdk.dir=D\:\\Android\\Sdk`). In the `android` folder (`gradlew.bat` on Windows):

| command | builds | result |
|---|---|---|
| `./gradlew assembleDebug` | the debug APK, package `com.github.dalerank.tutankhamun.debug` (installs beside the release one) | `android/app/build/outputs/apk/debug/app-debug.apk` |
| `./gradlew assembleRelease` | the release APK, package `com.github.dalerank.tutankhamun`, signed with the debug key | `android/app/build/outputs/apk/release/app-release.apk` |
| `./gradlew installDebug` | the debug APK, installed on the connected device (USB debugging on) | |

The APK holds `libmain.so` (the game, built by CMake from the top `CMakeLists.txt`), `libSDL2.so` and
`libSDL2_mixer.so` for arm64-v8a, armeabi-v7a and x86_64, and the `config` folder as assets, copied from the
repository at every build. The first build fetches SDL2 and SDL2_mixer and builds them for the three ABIs, so it takes
a while. An APK built elsewhere is signed with another debug key: uninstall the app before installing it
(`adb install -r <apk>` replaces one with the same key). The game's log is `tutankhamun_play.log` in the app's
storage; of the debug build it can be read with

    adb shell run-as com.github.dalerank.tutankhamun.debug cat files/tutankhamun_play.log

## Google Play

As in Julius, [Gradle Play Publisher](https://github.com/Triple-T/gradle-play-publisher) uploads the bundle signed
with the upload key; Google Play signs the installed app with its own key. Make the upload key once and keep it and
its passwords safe (a lost upload key can be reset through the Play Console, but it takes days):

    keytool -genkeypair -v -keystore tutankhamun-upload.jks -alias upload -keyalg RSA -keysize 2048 -validity 10000

Locally, `android/keystore.properties` (not in git) names it, and `android/play-publisher.json` (not in git) is the
JSON key of the Google Cloud service account that the Play Console allows to release the app:

    storeFile=/path/to/tutankhamun-upload.jks
    storePassword=...
    keyAlias=upload
    keyPassword=...

| command | does |
|---|---|
| `./gradlew bundleRelease` | the signed bundle `android/app/build/outputs/bundle/release/app-release.aab` |
| `./gradlew publishReleaseBundle` | builds it and uploads it to the internal testing track (`--track production`, `--release-status draft` change that) |
| `./gradlew publishReleaseListing` | uploads the store listing of `android/app/src/main/play`: the texts in English and Russian, the icon, the feature graphic |

`-PversionCode=<n>` sets the versionCode, which grows with every upload. The first upload of the app is made by hand
in the Play Console (the plugin cannot create the app). In the CI the repository's secrets `ANDROID_UPLOAD_KEYSTORE`
(the .jks in base64: `base64 -w0 tutankhamun-upload.jks`), `ANDROID_UPLOAD_STORE_PASSWORD`,
`ANDROID_UPLOAD_KEY_ALIAS` and `ANDROID_UPLOAD_KEY_PASSWORD` make the Android workflow build the bundle (the artifact
`android_aab`), and with `PLAY_SERVICE_ACCOUNT_JSON` (the service account's JSON key) every push to `main` publishes
it to internal testing with the commit's title as the release notes; the versionCode is the run number. The privacy
policy is [PRIVACY.md](../PRIVACY.md).
