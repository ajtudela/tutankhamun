// A C interface to Akhenaten's Bink decoder (bink.cpp), and the shim of its logs:: functions. Not Akhenaten code.

#include "bink_c.h"
#include "bink.h"
#include "core/log.h"

#include <cstdarg>
#include <cstdio>
#include <new>

static void (*log_function)(const char *format, ...);

static void log_line(const char *kind, const char *format, va_list args)
{
    if (!log_function) {
        return;
    }
    char text[512];
    std::vsnprintf(text, sizeof(text), format, args);
    log_function("%s%s", kind, text);
}

namespace logs {
void info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_line("", format, args);
    va_end(args);
}

void error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_line("error: ", format, args);
    va_end(args);
}
}

struct akh_bink {
    bink movie;
    int frames;
};

extern "C" {
void akh_bink_set_log(void (*log)(const char *format, ...))
{
    log_function = log;
}

akh_bink *akh_bink_open(const char *filename)
{
    unsigned char header[12];
    FILE *fp = std::fopen(filename, "rb");
    if (!fp) {
        return 0;
    }
    size_t got = std::fread(header, 1, sizeof(header), fp);
    std::fclose(fp);
    if (got != sizeof(header)) {
        return 0;
    }
    bink movie = bink_open(filename);
    if (!movie) {
        return 0;
    }
    akh_bink *b = new (std::nothrow) akh_bink;
    if (!b) {
        bink_close(movie);
        return 0;
    }
    b->movie = movie;
    b->frames = (int) (header[8] | (header[9] << 8) | (header[10] << 16) | ((unsigned) header[11] << 24));
    return b;
}

void akh_bink_close(akh_bink *b)
{
    if (b) {
        bink_close(b->movie);
        delete b;
    }
}

int akh_bink_width(akh_bink *b) { return b ? bink_get_width(b->movie) : 0; }
int akh_bink_height(akh_bink *b) { return b ? bink_get_height(b->movie) : 0; }
int akh_bink_frames(akh_bink *b) { return b ? b->frames : 0; }
int akh_bink_micros_per_frame(akh_bink *b) { return b ? bink_get_micros_per_frame(b->movie) : 0; }

int akh_bink_audio_channels(akh_bink *b)
{
    return b && bink_get_audio_sample_rate(b->movie) > 0 ? bink_get_audio_channels(b->movie) : 0;
}

int akh_bink_audio_rate(akh_bink *b) { return b ? bink_get_audio_sample_rate(b->movie) : 0; }
int akh_bink_first_frame(akh_bink *b) { return b ? bink_first_frame(b->movie) : 0; }
int akh_bink_next_frame(akh_bink *b) { return b ? bink_next_frame(b->movie) : 0; }
const uint32_t *akh_bink_frame_rgba(akh_bink *b) { return b ? bink_get_frame_rgba(b->movie) : 0; }

const void *akh_bink_frame_audio(akh_bink *b, int *bytes)
{
    *bytes = b ? bink_get_frame_audio_size(b->movie) : 0;
    return *bytes > 0 ? bink_get_frame_audio(b->movie) : 0;
}
}
