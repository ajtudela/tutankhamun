// The build's version (version.c): 0.1.<build>, the build being the number of commits of the source tree, its commit
// and date, given by CMake at configure time, as Akhenaten's version is made (CMakeLists.txt, "GAME_BUILD_NUMBER").

#ifndef PLATFORM_VERSION_H
#define PLATFORM_VERSION_H

#include <stdint.h>

int32_t version_build(void);
const char *version_commit(void);
// "Tutankhamun 0.1.<build> (<commit>) <date> <system>", for the front menu and the log
const char *version_text(void);

#endif
