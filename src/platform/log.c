// The run log: a text file opened once, one line per message.

#include "platform/log.h"
#include "platform/platform_file.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_BUFFER_SIZE (64 * 1024)
#define LOG_FLUSH_SECONDS 2

static FILE *log_file;
static char log_path[1024];
static char log_buffer[LOG_BUFFER_SIZE];
static time_t log_flushed_at;

static void log_buffer_attach(void)
{
    if (log_file) {
        setvbuf(log_file, log_buffer, _IOFBF, sizeof(log_buffer));
    }
    log_flushed_at = time(0);
}

static void log_close_at_exit(void)
{
    if (log_file) {
        fclose(log_file);
        log_file = 0;
    }
}

void platform_log_open(const char *path)
{
    log_file = fopen(path, "w");
    snprintf(log_path, sizeof(log_path), "%s", path);
    log_buffer_attach();
    static bool hooked;
    if (!hooked) {
        hooked = true;
        atexit(log_close_at_exit);
    }
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
    log_buffer_attach();
    platform_log("log: moved to %s", path);
    return true;
}

const char *platform_log_path(void)
{
    return log_file ? log_path : 0;
}

void platform_log_flush(void)
{
    if (log_file) {
        fflush(log_file);
        log_flushed_at = time(0);
    }
}

void platform_log_flush_due(void)
{
    if (log_file && time(0) - log_flushed_at >= LOG_FLUSH_SECONDS) {
        platform_log_flush();
    }
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
    platform_log_flush_due();
}
