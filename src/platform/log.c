// The run log: a text file opened once, one line per message, flushed at once so a crash keeps everything logged before
// it.

#include "platform/log.h"
#include "platform/platform_file.h"

#include <stdarg.h>
#include <stdio.h>

static FILE *log_file;
static char log_path[1024];

void platform_log_open(const char *path)
{
    log_file = fopen(path, "w");
    snprintf(log_path, sizeof(log_path), "%s", path);
}

bool platform_log_move(const char *path)
{
    FILE *moved = platform_fopen(path, "w");
    if (!moved) {
        platform_log("log: cannot move to %s", path);
        return false;
    }
    if (log_file) {
        fclose(log_file);
        FILE *old = fopen(log_path, "rb");
        if (old) {
            char buffer[4096];
            size_t got;
            while ((got = fread(buffer, 1, sizeof(buffer), old)) > 0) {
                fwrite(buffer, 1, got, moved);
            }
            fclose(old);
            remove(log_path);
        }
    }
    log_file = moved;
    snprintf(log_path, sizeof(log_path), "%s", path);
    platform_log("log: moved to %s", path);
    return true;
}

const char *platform_log_path(void)
{
    return log_file ? log_path : 0;
}

void platform_log(const char *format, ...)
{
    if (!log_file) {
        return;
    }
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fputc('\n', log_file);
    fflush(log_file);
}
