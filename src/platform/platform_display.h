// The display behind the game's graphics: the display the play mode registers (begin and shutdown of the game's
// display set-up go there), and the surfaces it lends. A surface is a frame buffer: locked for its pixels, pitch and
// size, unlocked to show what was drawn, filled with one colour, presented (the back surface onto the primary one),
// checked for loss and restored. A call on no surface answers PLATFORM_SURFACE_NONE.

#ifndef PLATFORM_DISPLAY_H
#define PLATFORM_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t *pixels;
    int32_t pitch;
    int32_t width;
    int32_t height;
} platform_frame;

enum platform_surface_result {
    PLATFORM_SURFACE_OK = 0,
    PLATFORM_SURFACE_LOST = 1,
    PLATFORM_SURFACE_FAILED = 2,
    PLATFORM_SURFACE_NONE = 3,
};

typedef struct platform_surface platform_surface;

typedef struct {
    int32_t (*lock)(platform_surface *surface, platform_frame *frame);
    int32_t (*unlock)(platform_surface *surface);
    int32_t (*fill)(platform_surface *surface, uint16_t colour);
    int32_t (*present)(platform_surface *surface, platform_surface *source);
    bool (*is_lost)(platform_surface *surface);
    int32_t (*restore)(platform_surface *surface);
} platform_surface_ops;

struct platform_surface {
    const platform_surface_ops *ops;
};

int32_t platform_surface_lock(platform_surface *surface, platform_frame *frame);
int32_t platform_surface_unlock(platform_surface *surface);
int32_t platform_surface_fill(platform_surface *surface, uint16_t colour);
int32_t platform_surface_present(platform_surface *surface, platform_surface *source);
bool platform_surface_is_lost(platform_surface *surface);
int32_t platform_surface_restore(platform_surface *surface);

void platform_display_set_host(int32_t (*begin)(void), int32_t (*shutdown)(int32_t release_display));
int32_t platform_display_replaced(void);
int32_t platform_display_begin(void);
int32_t platform_display_shutdown(int32_t release_display);

#endif
