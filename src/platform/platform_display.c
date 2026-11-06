// The display behind the game's graphics layer. The port has no display of its own: the play mode draws through its
// SDL window, so the primary and back surfaces stay null (present, restore and the lost-surface check then do
// nothing), except while a video plays, when they are the video player's surface.

#include "platform/platform_display.h"

#include <stddef.h>

int32_t platform_surface_lock(platform_surface *surface, platform_frame *frame)
{
    return surface ? surface->ops->lock(surface, frame) : PLATFORM_SURFACE_NONE;
}

int32_t platform_surface_unlock(platform_surface *surface)
{
    return surface ? surface->ops->unlock(surface) : PLATFORM_SURFACE_NONE;
}

int32_t platform_surface_fill(platform_surface *surface, uint16_t colour)
{
    return surface ? surface->ops->fill(surface, colour) : PLATFORM_SURFACE_NONE;
}

int32_t platform_surface_present(platform_surface *surface, platform_surface *source)
{
    return surface ? surface->ops->present(surface, source) : PLATFORM_SURFACE_NONE;
}

bool platform_surface_is_lost(platform_surface *surface)
{
    return surface && surface->ops->is_lost(surface);
}

int32_t platform_surface_restore(platform_surface *surface)
{
    return surface ? surface->ops->restore(surface) : PLATFORM_SURFACE_NONE;
}

static int32_t (*host_begin)(void);
static int32_t (*host_shutdown)(int32_t release_display);

void platform_display_set_host(int32_t (*begin)(void), int32_t (*shutdown)(int32_t release_display))
{
    host_begin = begin;
    host_shutdown = shutdown;
}

int32_t platform_display_replaced(void)
{
    return host_begin != NULL;
}

int32_t platform_display_begin(void)
{
    return host_begin ? host_begin() : 0;
}

int32_t platform_display_shutdown(int32_t release_display)
{
    return host_shutdown ? host_shutdown(release_display) : 0;
}
