cmake_minimum_required(VERSION 3.22)
project(tutankhamun C CXX)

# Tutankhamun: tutankhamun, the game of Pharaoh + Cleopatra in C. The game (src/game), the platform layer
# (src/platform) and the program around them, SDL2 and SDL2_mixer fetched and linked statically, Akhenaten's Bink
# decoder for the videos and parson for the JSON configs. Windows (MSVC, Win32 or x64) and Linux (GCC or Clang, 64-bit;
# the AppImage: packaging/linux).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    set(CMAKE_C_STANDARD 99)
else()
    set(CMAKE_C_STANDARD 11)
endif()
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4 /wd4100)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
    set(TUTANKHAMUN_THIRD_PARTY_WARNINGS /W1)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-missing-braces -fno-strict-aliasing)
    set(TUTANKHAMUN_THIRD_PARTY_WARNINGS -w)
endif()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS src/game/*.c)

# The platform: SDL2 window, input and SDL_mixer audio; static SDL2 + SDL2_mixer built once per build folder
# (cmake/TutankhamunBuildSDL.cmake; -DTUTANKHAMUN_SDL_LIBS_DIR=<dir> shares the built libraries between build folders).
add_library(tutankhamun_platform STATIC EXCLUDE_FROM_ALL
    src/platform/screen.c
    src/platform/sound_device.c
)
target_include_directories(tutankhamun_platform PUBLIC src PRIVATE src/game)

include(${CMAKE_SOURCE_DIR}/cmake/TutankhamunBuildSDL.cmake)
if(NOT TUTANKHAMUN_SDL_READY)
    message(FATAL_ERROR "SDL2 / SDL2_mixer did not become ready")
endif()
target_include_directories(tutankhamun_platform PUBLIC
    ${SDL2_INCLUDE_DIR}
    ${SDL2_INCLUDE_ADD_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
)
target_link_libraries(tutankhamun_platform PUBLIC
    ${SDL2_MIXER_LIBRARY}
    ${SDL2_LIBRARY}
)
if(SDL2_MAIN_LIBRARY)
    target_link_libraries(tutankhamun_platform PUBLIC ${SDL2_MAIN_LIBRARY})
endif()
if(WIN32)
    target_link_libraries(tutankhamun_platform PUBLIC winmm version imm32 setupapi)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(tutankhamun_platform PUBLIC Threads::Threads ${CMAKE_DL_LIBS} m)
endif()

# The videos: Bink 1 on Akhenaten's decoder (C++, src/third_party/akhenaten_bink, AGPL-3.0).
add_library(akhenaten_bink STATIC EXCLUDE_FROM_ALL
    src/third_party/akhenaten_bink/bink.cpp
    src/third_party/akhenaten_bink/bink_c.cpp
)
set_target_properties(akhenaten_bink PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(akhenaten_bink PRIVATE src/third_party/akhenaten_bink)
target_compile_options(akhenaten_bink PRIVATE ${TUTANKHAMUN_THIRD_PARTY_WARNINGS})
target_link_libraries(akhenaten_bink PRIVATE tutankhamun_platform)

# The gameplay configs: parson 1.5.3 (C, MIT, src/third_party/parson).
add_library(parson STATIC src/third_party/parson/parson.c)
target_compile_options(parson PRIVATE ${TUTANKHAMUN_THIRD_PARTY_WARNINGS})

if(WIN32)
    set(TUTANKHAMUN_SYSTEM_SOURCES res/tutankhamun.rc src/app/port.c src/platform/platform_win32.c)
else()
    set(TUTANKHAMUN_SYSTEM_SOURCES src/app/port_posix.c src/platform/platform_posix.c)
endif()

add_executable(tutankhamun
    ${TUTANKHAMUN_SYSTEM_SOURCES}
    src/app/app_files.c
    src/app/app_icon.c
    src/app/game_dir.c
    src/app/main.c
    src/app/media_audio.c
    src/app/media_video.c
    src/app/miles_sdl.c
    src/app/play.c
    src/app/play_cursors.c
    src/app/startup.c
    src/app/video_bink.c
    src/platform/cursor.c
    src/platform/discord_rpc.c
    src/platform/log.c
    src/platform/platform_display.c
    src/platform/platform_file.c
    src/platform/platform_screen.c
    ${GAME_SOURCES}
)
target_include_directories(tutankhamun PRIVATE src src/game)
# a call that does not match its prototype, or a call without one, is an error
if(MSVC)
    target_compile_options(tutankhamun PRIVATE /we4113 /we4028 /we4047 /we4024 /we4133 /we4020 /we4029 /we4021 /we4013)
    target_link_options(tutankhamun PRIVATE /DYNAMICBASE /INCREMENTAL:NO)
    target_link_libraries(tutankhamun PRIVATE winmm)
else()
    target_compile_options(tutankhamun PRIVATE -Werror=implicit-function-declaration -Werror=int-conversion -Werror=incompatible-pointer-types
        -Werror=return-type -Werror=pointer-to-int-cast -Werror=int-to-pointer-cast)
    # the C++ run time of the Bink decoder is linked in: the program needs only the system's C library
    target_link_options(tutankhamun PRIVATE -static-libstdc++ -static-libgcc -rdynamic)
endif()
target_compile_definitions(tutankhamun PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(tutankhamun PRIVATE parson tutankhamun_platform akhenaten_bink)
set_target_properties(tutankhamun PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# The gameplay configs (config/*.json) are copied beside the program on every build.
add_custom_target(tutankhamun_configs COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/config" "$<TARGET_FILE_DIR:tutankhamun>/config")
add_dependencies(tutankhamun tutankhamun_configs)

if(WIN32)
    # The game folder (`cmake --install`, build.bat): tutankhamun.exe and config\ beside it.
    install(TARGETS tutankhamun RUNTIME DESTINATION .)
    install(DIRECTORY config/ DESTINATION config)
else()
    # The Linux tree (`cmake --install` with a prefix, the AppDir of the AppImage): bin/tutankhamun, the configs in
    # share/tutankhamun/config, the desktop entry, the icons and the AppStream metadata.
    include(GNUInstallDirs)
    install(TARGETS tutankhamun RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY config/ DESTINATION ${CMAKE_INSTALL_DATADIR}/tutankhamun/config)
    install(FILES packaging/linux/tutankhamun.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES packaging/linux/tutankhamun.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(FILES res/tutankhamun_256.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps RENAME tutankhamun.png)
    install(FILES res/tutankhamun.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME tutankhamun.svg)
endif()
