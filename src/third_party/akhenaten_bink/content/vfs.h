// Shim for Akhenaten's content/vfs.h, as much of it as bink.cpp uses: file_exists and path(...).resolve() on plain
// file system paths (src/app/video_bink.c has already mapped the game's path to a file), and open_read, which also
// takes "fd:N", a file descriptor the caller keeps open (on Android, a file of the game folder that has no path the app
// may open), read through a duplicate of it from the start. Not Akhenaten code.
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace vfs {
inline FILE *open_read(const char *filename)
{
#if !defined(_WIN32)
    if (std::strncmp(filename, "fd:", 3) == 0) {
        int fd = dup(std::atoi(filename + 3));
        if (fd < 0) {
            return nullptr;
        }
        FILE *fp = fdopen(fd, "rb");
        if (!fp) {
            close(fd);
            return nullptr;
        }
        std::fseek(fp, 0, SEEK_SET);
        return fp;
    }
#endif
    return std::fopen(filename, "rb");
}

inline bool file_exists(const char *filename)
{
    FILE *fp = open_read(filename);
    if (!fp) {
        return false;
    }
    std::fclose(fp);
    return true;
}

class path {
public:
    explicit path(const char *p) : p_(p ? p : "") {}
    path resolve() const { return file_exists(p_.c_str()) ? *this : path(""); }
    bool empty() const { return p_.empty(); }
    const char *c_str() const { return p_.c_str(); }

private:
    std::string p_;
};
}
