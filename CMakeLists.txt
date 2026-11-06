cmake_minimum_required(VERSION 3.22)
project(tutankhamun C CXX)

# Tutankhamun: tutankhamun, the game of Pharaoh + Cleopatra in C. The game (src/game), the platform layer
# (src/platform) and the program around them, SDL2 and SDL2_mixer fetched and linked statically, Akhenaten's Bink
# decoder for the videos and parson for the JSON configs.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "tutankhamun is a 32-bit program: configure with -A Win32")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options(/W4 /wd4100)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)

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
target_link_libraries(tutankhamun_platform PUBLIC winmm version imm32 setupapi)

# The videos: Bink 1 on Akhenaten's decoder (C++, src/third_party/akhenaten_bink, AGPL-3.0).
add_library(akhenaten_bink STATIC EXCLUDE_FROM_ALL
    src/third_party/akhenaten_bink/bink.cpp
    src/third_party/akhenaten_bink/bink_c.cpp
)
set_target_properties(akhenaten_bink PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(akhenaten_bink PRIVATE src/third_party/akhenaten_bink)
target_compile_options(akhenaten_bink PRIVATE /W1)
target_link_libraries(akhenaten_bink PRIVATE tutankhamun_platform)

# The gameplay configs: parson 1.5.3 (C, MIT, src/third_party/parson).
add_library(parson STATIC src/third_party/parson/parson.c)
target_compile_options(parson PRIVATE /W1)

add_executable(tutankhamun
    res/tutankhamun.rc
    src/app/app_icon.c
    src/app/game_dir.c
    src/app/main.c
    src/app/media_audio.c
    src/app/media_video.c
    src/app/miles_sdl.c
    src/app/play.c
    src/app/play_cursors.c
    src/app/port.c
    src/app/startup.c
    src/app/video_bink.c
    src/platform/cursor.c
    src/platform/discord_rpc.c
    src/platform/log.c
    src/platform/platform_display.c
    src/platform/platform_screen.c
    src/platform/platform_win32.c
    ${GAME_SOURCES}
)
target_include_directories(tutankhamun PRIVATE src src/game)
# a call that does not match its prototype, or a call without one, is an error
target_compile_options(tutankhamun PRIVATE /we4113 /we4028 /we4047 /we4024 /we4133 /we4020 /we4029 /we4021 /we4013)
target_compile_definitions(tutankhamun PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(tutankhamun PRIVATE winmm parson tutankhamun_platform akhenaten_bink)
target_link_options(tutankhamun PRIVATE /DYNAMICBASE /INCREMENTAL:NO)
set_target_properties(tutankhamun PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# The gameplay configs (config/*.json) are copied beside the program on every build.
add_custom_target(tutankhamun_configs COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/config" "$<TARGET_FILE_DIR:tutankhamun>/config")
add_dependencies(tutankhamun tutankhamun_configs)

# The game folder (`cmake --install`, build.bat): tutankhamun.exe and config\ beside it.
install(TARGETS tutankhamun RUNTIME DESTINATION .)
install(DIRECTORY config/ DESTINATION config)
