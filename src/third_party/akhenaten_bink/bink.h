// Akhenaten's Bink 1 (BIKf) decoder, copied unchanged below this comment from
//     https://github.com/dalerank/Akhenaten  src/io/gamefiles/bink.h
//     commit 3978bfeea6d4e3ce5cd02220636d3e5985197f1d (2026-10-12)
// Copyright (C) the Akhenaten authors. Licensed under the GNU Affero General Public License v3.0 (AGPL-3.0), the
// licence of this project too (LICENSE at the top of the repository).
// Upstream history: added by mandel99 in Akhenaten PR #575 (d0f2e386, 2026-06-05), unchanged since. The upstream
// file has no copyright line of its own.
// Its two dependencies, core/log.h and content/vfs.h, are small shims in this folder; src/app/video_bink.c
// uses it through bink_c.h.

#pragma once

#include <stdint.h>

struct bink_movie;
using bink = bink_movie*;
using bink_audio_sink = void (*)(void* userdata, const void* data, int len);

bink bink_open(const char* filename);
void bink_close(bink movie);

int bink_get_width(bink movie);
int bink_get_height(bink movie);
int bink_get_micros_per_frame(bink movie);

int bink_has_audio(bink movie);
int bink_get_audio_channels(bink movie);
int bink_get_audio_sample_rate(bink movie);
int bink_get_audio_bitdepth(bink movie);

int bink_first_frame(bink movie);
int bink_next_frame(bink movie);
int bink_prebuffer_frames(bink movie, int frame_count, bink_audio_sink sink, void* userdata);
int bink_current_audio_is_prebuffered(bink movie);

const uint32_t* bink_get_frame_rgba(bink movie);
const void* bink_get_frame_audio(bink movie);
int bink_get_frame_audio_size(bink movie);
