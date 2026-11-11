// The run log: a text file opened once, one line per message, flushed at once so a crash keeps everything logged before
// it.

#include "platform/log.h"

#include <stdarg.h>
#include <stdio.h>

static FILE *log_file;
static char log_path[1024];

void platform_log_open(const char *path)
{
    log_file = fopen(path, "w");
    snprintf(log_path, sizeof(log_path), "%s", path);
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
