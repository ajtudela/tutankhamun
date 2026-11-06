// A C interface to Akhenaten's Bink decoder (bink.h is C++), for src/app/video_bink.c. Not Akhenaten code.
#ifndef AKHENATEN_BINK_C_H
#define AKHENATEN_BINK_C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct akh_bink akh_bink;

void akh_bink_set_log(void (*log)(const char *format, ...));

akh_bink *akh_bink_open(const char *filename);
void akh_bink_close(akh_bink *movie);

int akh_bink_width(akh_bink *movie);
int akh_bink_height(akh_bink *movie);
int akh_bink_frames(akh_bink *movie);
int akh_bink_micros_per_frame(akh_bink *movie);
int akh_bink_audio_channels(akh_bink *movie);
int akh_bink_audio_rate(akh_bink *movie);

int akh_bink_first_frame(akh_bink *movie);
int akh_bink_next_frame(akh_bink *movie);

const uint32_t *akh_bink_frame_rgba(akh_bink *movie);
const void *akh_bink_frame_audio(akh_bink *movie, int *bytes);

#ifdef __cplusplus
}
#endif

#endif
