// The play mode's cursors: the play build's arrow, shovel and sword (platform/cursor.c) at the --cursor-scale scale,
// written as handles into the game's cursor slots; each game cursor shape shows the closest of the three, as
// Akhenaten picks them (the sword for the legions' and ships' orders, the shovel for clearing land, else the arrow).
// The handles are the values the headless runs write, so the game's memory is the same in both.

#include "app/play_cursors.h"

#include "input/mouse.h"
#include "platform/cursor.h"
#include "window/window_data.h"

enum play_cursor_handle {
    PLAY_CURSOR_HANDLE_BASE = 0xC000,
};

typedef struct {
    uint16_t resource;
    enum cursor_kind kind;
} shape_cursor;

static const shape_cursor shape_cursors[CURSOR_SHAPES] = {
    [CURSOR_SHAPE_ARROW] = { 0x66, CURSOR_KIND_ARROW },
    [CURSOR_SHAPE_LEGION_MOVE] = { 0x96, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_CLEAR_LAND] = { 0x82, CURSOR_KIND_SHOVEL },
    [CURSOR_SHAPE_SHIP_MOVE] = { 0x8E, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_SHIP_LAND] = { 0x90, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_SHIP_EMBARK] = { 0x91, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_TRANSPORT_DOCKED] = { 0x92, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_WARSHIP_DOCKED] = { 0x93, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_INFANTRY_ATTACK] = { 0x88, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_ATTACK] = { 0x94, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LEGION_ORDER] = { 0x95, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_1] = { 0x98, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_2] = { 0x99, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LAYOUT_SINGLE_LINE_1] = { 0x9A, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LAYOUT_SINGLE_LINE_2] = { 0x9B, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_LAYOUT_CHARGE] = { 0x9C, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_TRANSPORT_EVADING] = { 0x9D, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_WARSHIP_HUNTING] = { 0x9F, CURSOR_KIND_SWORD },
    [CURSOR_SHAPE_SHIP_BLOCKED] = { 0xA0, CURSOR_KIND_SWORD },
};

static int32_t cursor_scale_percent = CURSOR_SCALE_PERCENT_MIN;

void play_cursors_set_scale(int32_t scale_percent)
{
    cursor_scale_percent = scale_percent;
}

int32_t play_cursors_install(void)
{
    for (int32_t shape = 0; shape < CURSOR_SHAPES; shape++) {
        cursor_handles[cursor_slot_of_shape[shape]] = (void *) (uintptr_t) (PLAY_CURSOR_HANDLE_BASE + shape_cursors[shape].resource);
    }
    return platform_cursors_create(cursor_scale_percent);
}

void play_cursor_show(void *handle)
{
    enum cursor_kind kind = CURSOR_KINDS;
    for (int32_t shape = 0; shape < CURSOR_SHAPES; shape++) {
        if ((uintptr_t) handle == (uintptr_t) (PLAY_CURSOR_HANDLE_BASE + shape_cursors[shape].resource)) {
            kind = shape_cursors[shape].kind;
        }
    }
    platform_cursor_kind_show(kind);
}
