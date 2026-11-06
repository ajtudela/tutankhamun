// The play build's own mouse cursors (cursor.c): platform_cursors_create makes the SDL colour cursors once at
// start-up; platform_cursor_kind_show shows one of them, the system's arrow for a kind it has none of.
// As Akhenaten has them (https://github.com/dalerank/Akhenaten src/input/cursor.h, src/platform/cursor.h).
// Copyright (C) the Akhenaten authors. Licence: GNU Affero General Public License v3.0 (LICENSES/AGPL-3.0.txt).

#ifndef PLATFORM_CURSOR_H
#define PLATFORM_CURSOR_H

#include <stdint.h>

enum cursor_kind {
    CURSOR_KIND_ARROW = 0,
    CURSOR_KIND_SHOVEL = 1,
    CURSOR_KIND_SWORD = 2,
    CURSOR_KINDS = 3,
};

enum cursor_scale {
    CURSOR_SCALE_1 = 0,
    CURSOR_SCALE_1_5 = 1,
    CURSOR_SCALE_2 = 2,
    CURSOR_SCALES = 3,
};

enum cursor_scale_percent {
    CURSOR_SCALE_PERCENT_MIN = 100,
    CURSOR_SCALE_PERCENT_MAX = 200,
};

enum cursor_scale cursor_scale_for(int32_t scale_percent);
int32_t platform_cursors_create(int32_t scale_percent);
void platform_cursor_kind_show(enum cursor_kind kind);

#endif
