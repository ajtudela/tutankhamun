// The run log: a text file opened once, one line per message, flushed at once so a crash keeps everything logged before
// it.

#include "platform/log.h"

#include <stdarg.h>
#include <stdio.h>

static FILE *log_file;

void platform_log_open(const char *path)
{
    log_file = fopen(path, "w");
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
