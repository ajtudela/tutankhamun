/* Modelled on Julius src/platform/screen.c (https://github.com/bvschaik/julius, compared at commit
 * 34d1ecd54befb845c0139b371fa8a0438210dac1, 2026-09-12). Licence: GNU Affero General Public License v3.0
 * (LICENSES/AGPL-3.0.txt). The upstream file has no copyright line of its own (copyright: the Julius authors).
 * Taken from Julius: the function names platform_screen_create / _destroy / _resize / _clear / _update / _render
 * and their shape (one SDL window, renderer and streaming texture; update = clear and copy the texture; render =
 * present). The bodies are this project's: no Julius config, cursor, scaling, fullscreen or platform
 * branches; the game's 16-bit screen buffer converted to ARGB8888 and uploaded in platform_screen_update. */

#include "platform/screen.h"

#include "platform/platform_screen.h"

#include <stdlib.h>
#include <string.h>
#include "SDL.h"

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *argb;
    int32_t width;
    int32_t height;
} sdl;

static void destroy_texture(void)
{
    if (sdl.texture) {
        SDL_DestroyTexture(sdl.texture);
        sdl.texture = 0;
    }
    free(sdl.argb);
    sdl.argb = 0;
}

int32_t platform_screen_create(const char *title, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        width = 640;
        height = 480;
    }

    platform_screen_destroy();

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return 0;
    }

    sdl.window = SDL_CreateWindow(title ? title : "Tutankhamun", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
    if (!sdl.window) {
        return 0;
    }

    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!sdl.renderer) {
        sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!sdl.renderer) {
        platform_screen_destroy();
        return 0;
    }

    return platform_screen_resize(width, height);
}

void platform_screen_destroy(void)
{
    destroy_texture();
    if (sdl.renderer) {
        SDL_DestroyRenderer(sdl.renderer);
        sdl.renderer = 0;
    }
    if (sdl.window) {
        SDL_DestroyWindow(sdl.window);
        sdl.window = 0;
    }
    sdl.width = 0;
    sdl.height = 0;
}

int32_t platform_screen_resize(int32_t width, int32_t height)
{
    if (!sdl.renderer || width < 1 || height < 1) {
        return 0;
    }

    destroy_texture();
    SDL_RenderSetLogicalSize(sdl.renderer, width, height);

    sdl.texture = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!sdl.texture) {
        return 0;
    }

    sdl.argb = (uint32_t *) malloc((size_t) width * (size_t) height * sizeof(uint32_t));
    if (!sdl.argb) {
        destroy_texture();
        return 0;
    }

    sdl.width = width;
    sdl.height = height;
    return 1;
}

void platform_screen_clear(void)
{
    if (sdl.renderer) {
        SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl.renderer);
    }
}

void platform_screen_update(void)
{
    const uint16_t *src = platform_screen_buffer();
    int32_t w = platform_screen_width();
    int32_t h = platform_screen_height();
    int32_t x, y;

    if (!sdl.renderer || !sdl.texture || !sdl.argb || !src || w < 1 || h < 1) {
        return;
    }

    if (w != sdl.width || h != sdl.height) {
        if (!platform_screen_resize(w, h)) {
            return;
        }
    }

    int32_t is_565 = platform_screen_is_565();
    for (y = 0; y < h; y++) {
        const uint16_t *row = src + (size_t) y * (size_t) w;
        uint32_t *dst = sdl.argb + (size_t) y * (size_t) w;
        for (x = 0; x < w; x++) {
            dst[x] = is_565 ? platform_rgb565_to_argb8888(row[x]) : platform_rgb555_to_argb8888(row[x]);
        }
    }

    SDL_UpdateTexture(sdl.texture, NULL, sdl.argb, w * (int32_t) sizeof(uint32_t));
    SDL_RenderClear(sdl.renderer);
    SDL_RenderCopy(sdl.renderer, sdl.texture, NULL, NULL);
}

void platform_screen_render(void)
{
    if (sdl.renderer) {
        SDL_RenderPresent(sdl.renderer);
    }
}

void *platform_screen_window(void)
{
    return sdl.window;
}

