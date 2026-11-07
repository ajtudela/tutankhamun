# Fetch + static-build SDL2 / SDL2_mixer (Akhenaten pattern).
#
# Adapted from Akhenaten CMakeLists.txt, its "SDL2 section" and "SDL2_mixer section" (lines 366-545 at
# https://github.com/dalerank/Akhenaten commit 3978bfeea6d4e3ce5cd02220636d3e5985197f1d, 2026-10-12; those lines
# written by dalerank, mostly in e2ef321a, 063f897f, fd8d89d2). Licence: GNU Affero General Public License v3.0,
# (LICENSES/AGPL-3.0.txt). Upstream has no copyright line of its own (copyright: the Akhenaten authors).
# Kept from upstream: the versions, the FetchContent declarations and the local _deps override, the prebuilt_libs /
# libs directory layout, the execute_process call of BuildSDL2*.cmake with its arguments, the library names.
# Changed here: one file of its own, a build once per build directory, Debug / Release libraries chosen by
# generator expression (upstream: by CMAKE_BUILD_TYPE), the generator / platform / toolset passed on, no
# Emscripten branch, no CACHE FORCE variables, TUTANKHAMUN_SDL_LIBS_DIR (the installed libraries in a folder that several
# build folders can share, under a lock; nothing is fetched when they are there).
# Included from CMakeLists.txt.
#
# Sets (on success):
#   TUTANKHAMUN_SDL_READY, SDL2_LIBRARY, SDL2_MAIN_LIBRARY, SDL2_INCLUDE_DIR,
#   SDL2_MIXER_LIBRARY, SDL2_MIXER_INCLUDE_DIR, SDL2_ROOT

include(FetchContent)

set(TUTANKHAMUN_SDL2_VERSION "2.32.10")
set(TUTANKHAMUN_SDL2_MIXER_VERSION "2.8.0")

set(PREBUILT_LIBS_INTERMEDIATE_DIR "${CMAKE_BINARY_DIR}/prebuilt_libs"
    CACHE PATH "Directory for intermediate SDL builds")
set(BUILD_ADDITIONAL_CMAKE_ARGS "")
# The nested SDL builds are multi-config (Debug and Release, BuildSDL2*.cmake): under the single-config Ninja of the
# Ninja presets (vcvars32) they use Ninja Multi-Config, which lays the libraries out per config as Visual Studio does.
set(TUTANKHAMUN_SDL_GENERATOR "${CMAKE_GENERATOR}")
if(MSVC AND CMAKE_GENERATOR STREQUAL "Ninja")
    set(TUTANKHAMUN_SDL_GENERATOR "Ninja Multi-Config")
endif()
# macOS: the nested SDL builds get the program's architectures (arm64;x86_64 for the universal app) and oldest system
# through the environment, which CMake reads into CMAKE_OSX_ARCHITECTURES / CMAKE_OSX_DEPLOYMENT_TARGET; the list's
# semicolon would not survive the command line of execute_process.
if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES)
        set(ENV{CMAKE_OSX_ARCHITECTURES} "${CMAKE_OSX_ARCHITECTURES}")
    endif()
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        set(ENV{MACOSX_DEPLOYMENT_TARGET} "${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif()

set(LOCAL_DEPS_ROOT "${CMAKE_SOURCE_DIR}/_deps")
set(TUTANKHAMUN_SDL_LIBS_DIR "${CMAKE_BINARY_DIR}/libs" CACHE PATH "The folder of the installed static SDL2 and SDL2_mixer")
file(MAKE_DIRECTORY "${TUTANKHAMUN_SDL_LIBS_DIR}")
file(LOCK "${TUTANKHAMUN_SDL_LIBS_DIR}/build.lock" GUARD FILE TIMEOUT 3600)

# --- SDL2 ---
set(SDL2_INSTALL_DIR "${TUTANKHAMUN_SDL_LIBS_DIR}/SDL2")
# Built once per libraries folder: a re-configure keeps the libraries already installed
if(NOT EXISTS "${SDL2_INSTALL_DIR}/SDL2-staticd.lib" AND NOT EXISTS "${SDL2_INSTALL_DIR}/libSDL2-static.a")
if(EXISTS "${LOCAL_DEPS_ROOT}/SDL2-src/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_SDL2 "${LOCAL_DEPS_ROOT}/SDL2-src")
    set(FETCHCONTENT_UPDATES_DISCONNECTED_SDL2 TRUE)
    message(STATUS "SDL: using local _deps/SDL2-src")
endif()

message(STATUS "SDL: fetching SDL2 release-${TUTANKHAMUN_SDL2_VERSION} (may take a while)...")
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-${TUTANKHAMUN_SDL2_VERSION}
)
FetchContent_GetProperties(SDL2)
if(NOT SDL2_POPULATED)
    FetchContent_Populate(SDL2)
endif()

set(SDL2_SOURCE_DIR "${sdl2_SOURCE_DIR}")
set(SDL2_BUILD_BASE_DIR "${PREBUILT_LIBS_INTERMEDIATE_DIR}/SDL2")
set(SDL2_BUILD_DIR "${SDL2_BUILD_BASE_DIR}/build")
file(MAKE_DIRECTORY ${SDL2_BUILD_BASE_DIR})
file(MAKE_DIRECTORY ${SDL2_BUILD_DIR})
file(MAKE_DIRECTORY ${SDL2_INSTALL_DIR})

message(STATUS "SDL: building static SDL2 into ${SDL2_INSTALL_DIR}...")
set(SDL2_BUILD_CMAKE_ARGS
    -DSDL2_VERSION=${TUTANKHAMUN_SDL2_VERSION}
    -DSDL2_SOURCE_DIR=${SDL2_SOURCE_DIR}
    -DSDL2_BUILD_DIR=${SDL2_BUILD_DIR}
    -DSDL2_INSTALL_DIR=${SDL2_INSTALL_DIR}
    -DSDL2_BUILD_CONFIG=${CMAKE_BUILD_TYPE}
    -DSDL2_IS_MSVC=${MSVC}
    -DSDL2_ADDITIONAL_CMAKE_ARGS=${BUILD_ADDITIONAL_CMAKE_ARGS}
    -DSDL2_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSDL2_GENERATOR=${TUTANKHAMUN_SDL_GENERATOR}
    -DSDL2_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
    -DSDL2_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
)
execute_process(
    COMMAND ${CMAKE_COMMAND} ${SDL2_BUILD_CMAKE_ARGS} -P ${CMAKE_SOURCE_DIR}/cmake/BuildSDL2.cmake
    WORKING_DIRECTORY ${SDL2_BUILD_BASE_DIR}
    RESULT_VARIABLE SDL2_BUILD_RESULT
)
if(NOT SDL2_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "SDL: SDL2 build failed (exit ${SDL2_BUILD_RESULT})")
endif()
endif()

if(MSVC)
    # Multi-config (Visual Studio): the Debug build links the /MTd libraries, the others the /MT ones.
    set(SDL2_LIBRARY "$<IF:$<CONFIG:Debug>,${SDL2_INSTALL_DIR}/SDL2-staticd.lib,${SDL2_INSTALL_DIR}/SDL2-static.lib>")
    set(SDL2_MAIN_LIBRARY "$<IF:$<CONFIG:Debug>,${SDL2_INSTALL_DIR}/SDL2maind.lib,${SDL2_INSTALL_DIR}/SDL2main.lib>")
else()
    set(SDL2_LIBRARY "${SDL2_INSTALL_DIR}/libSDL2-static.a")
    set(SDL2_MAIN_LIBRARY "")
endif()

set(SDL2_INCLUDE_DIR "${SDL2_INSTALL_DIR}/include/SDL2")
set(SDL2_INCLUDE_ADD_DIR "${SDL2_INSTALL_DIR}/include")
set(SDL2_ROOT "${SDL2_INSTALL_DIR}")
set(CMAKE_PREFIX_PATH "${SDL2_ROOT};${CMAKE_PREFIX_PATH}")
set(SDL2_DIR "${SDL2_INSTALL_DIR}/lib/cmake/SDL2")

# --- SDL2_mixer ---
set(SDL2_MIXER_INSTALL_DIR "${TUTANKHAMUN_SDL_LIBS_DIR}/SDL2_mixer")
if(NOT EXISTS "${SDL2_MIXER_INSTALL_DIR}/SDL2_mixer-staticd.lib" AND NOT EXISTS "${SDL2_MIXER_INSTALL_DIR}/libSDL2_mixer-static.a")
if(EXISTS "${LOCAL_DEPS_ROOT}/SDL2_mixer-src/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_SDL2_MIXER "${LOCAL_DEPS_ROOT}/SDL2_mixer-src")
    set(FETCHCONTENT_UPDATES_DISCONNECTED_SDL2_MIXER TRUE)
    message(STATUS "SDL: using local _deps/SDL2_mixer-src")
endif()

message(STATUS "SDL: fetching SDL2_mixer release-${TUTANKHAMUN_SDL2_MIXER_VERSION}...")
FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-${TUTANKHAMUN_SDL2_MIXER_VERSION}
)
FetchContent_GetProperties(SDL2_mixer)
if(NOT SDL2_mixer_POPULATED)
    FetchContent_Populate(SDL2_mixer)
endif()

set(SDL2_MIXER_SOURCE_DIR "${sdl2_mixer_SOURCE_DIR}")
set(SDL2_MIXER_BUILD_BASE_DIR "${PREBUILT_LIBS_INTERMEDIATE_DIR}/SDL2_mixer")
set(SDL2_MIXER_BUILD_DIR "${SDL2_MIXER_BUILD_BASE_DIR}/build")
file(MAKE_DIRECTORY ${SDL2_MIXER_BUILD_BASE_DIR})
file(MAKE_DIRECTORY ${SDL2_MIXER_BUILD_DIR})
file(MAKE_DIRECTORY ${SDL2_MIXER_INSTALL_DIR})

message(STATUS "SDL: building static SDL2_mixer into ${SDL2_MIXER_INSTALL_DIR}...")
set(SDL2_MIXER_BUILD_CMAKE_ARGS
    -DSDL2_MIXER_VERSION=${TUTANKHAMUN_SDL2_MIXER_VERSION}
    -DSDL2_MIXER_SOURCE_DIR=${SDL2_MIXER_SOURCE_DIR}
    -DSDL2_MIXER_BUILD_DIR=${SDL2_MIXER_BUILD_DIR}
    -DSDL2_MIXER_INSTALL_DIR=${SDL2_MIXER_INSTALL_DIR}
    -DSDL2_MIXER_BUILD_CONFIG=${CMAKE_BUILD_TYPE}
    -DSDL2_MIXER_IS_MSVC=${MSVC}
    -DSDL2_ROOT=${SDL2_ROOT}
    -DSDL2_MIXER_ADDITIONAL_CMAKE_ARGS=${BUILD_ADDITIONAL_CMAKE_ARGS}
    -DSDL2_MIXER_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSDL2_MIXER_GENERATOR=${TUTANKHAMUN_SDL_GENERATOR}
    -DSDL2_MIXER_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
    -DSDL2_MIXER_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
)
execute_process(
    COMMAND ${CMAKE_COMMAND} ${SDL2_MIXER_BUILD_CMAKE_ARGS} -P ${CMAKE_SOURCE_DIR}/cmake/BuildSDL2_mixer.cmake
    WORKING_DIRECTORY ${SDL2_MIXER_BUILD_BASE_DIR}
    RESULT_VARIABLE SDL2_MIXER_BUILD_RESULT
)
if(NOT SDL2_MIXER_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "SDL: SDL2_mixer build failed (exit ${SDL2_MIXER_BUILD_RESULT})")
endif()
endif()

if(MSVC)
    set(SDL2_MIXER_LIBRARY
        "$<IF:$<CONFIG:Debug>,${SDL2_MIXER_INSTALL_DIR}/SDL2_mixer-staticd.lib,${SDL2_MIXER_INSTALL_DIR}/SDL2_mixer-static.lib>")
else()
    set(SDL2_MIXER_LIBRARY "${SDL2_MIXER_INSTALL_DIR}/libSDL2_mixer-static.a")
endif()

if(EXISTS "${SDL2_MIXER_INSTALL_DIR}/include/SDL2/SDL_mixer.h")
    set(SDL2_MIXER_INCLUDE_DIR "${SDL2_MIXER_INSTALL_DIR}/include/SDL2")
elseif(EXISTS "${SDL2_MIXER_INSTALL_DIR}/include/SDL_mixer.h")
    set(SDL2_MIXER_INCLUDE_DIR "${SDL2_MIXER_INSTALL_DIR}/include")
else()
    set(SDL2_MIXER_INCLUDE_DIR "${SDL2_MIXER_INSTALL_DIR}/include/SDL2")
endif()

file(LOCK "${TUTANKHAMUN_SDL_LIBS_DIR}/build.lock" RELEASE)
set(TUTANKHAMUN_SDL_READY TRUE)
message(STATUS "SDL: ready (SDL2=${SDL2_LIBRARY}, mixer=${SDL2_MIXER_LIBRARY})")
