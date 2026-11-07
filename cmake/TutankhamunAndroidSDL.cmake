# SDL2 and SDL2_mixer for the Android build (android/, the Gradle project): the same versions as the desktop builds
# (TutankhamunBuildSDL.cmake), fetched and built as subprojects of the game's CMake run, shared: SDLActivity loads
# libSDL2.so, libSDL2_mixer.so and the game's libmain.so. The mixer has the same codecs as on the desktop: WAV and
# MP3 (minimp3).
#
# Sets TUTANKHAMUN_SDL_READY and the targets SDL2::SDL2 and SDL2_mixer::SDL2_mixer.

include(FetchContent)

set(TUTANKHAMUN_SDL2_VERSION "2.32.10")
set(TUTANKHAMUN_SDL2_MIXER_VERSION "2.8.0")

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

set(SDL2MIXER_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVE ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3_MINIMP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3_MPG123 OFF CACHE BOOL "" FORCE)
foreach(codec FLAC MOD MIDI OPUS VORBIS WAVPACK GME CMD)
    set(SDL2MIXER_${codec} OFF CACHE BOOL "" FORCE)
endforeach()

FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-${TUTANKHAMUN_SDL2_VERSION}
    GIT_SHALLOW TRUE
)
FetchContent_Declare(SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-${TUTANKHAMUN_SDL2_MIXER_VERSION}
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2 SDL2_mixer)

if(TARGET SDL2::SDL2 AND TARGET SDL2_mixer::SDL2_mixer)
    set(TUTANKHAMUN_SDL_READY TRUE)
endif()
