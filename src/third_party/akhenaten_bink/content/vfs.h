// Shim for Akhenaten's content/vfs.h, as much of it as bink.cpp uses: file_exists and path(...).resolve() on plain
// file system paths (src/app/video_bink.c has already mapped the game's path to a file). Not Akhenaten code.
#pragma once

#include <cstdio>
#include <string>

namespace vfs {
inline bool file_exists(const char *filename)
{
    FILE *fp = std::fopen(filename, "rb");
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
