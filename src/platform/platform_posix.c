// The platform functions on Linux and the other POSIX systems, on SDL. The clocks, the keyboard and cursor state, the
// message pump and the message box call the function the front end bound to them (platform_bind), or else an answer of
// their own: there is no system message queue, so an unbound peek finds no message. The game's window is the SDL window
// (platform_main_window); a document or a web address opens with xdg-open (open on macOS, SDL_OpenURL on Android), a
// web address of the main menu's links only when the play mode allowed the links. The desktop text of the loading
// screen is not drawn.

#include "platform/platform_win32.h"

#include "platform/platform_dialog.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_shell.h"
#include "platform/platform_system.h"
#include "platform/platform_time.h"
#include "platform/platform_window.h"

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static uint32_t time_ms_posix(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t) ((uint64_t) now.tv_sec * 1000u + (uint64_t) now.tv_nsec / 1000000u);
}

static int16_t key_state_posix(int32_t key)
{
    return 0;
}

static bool cursor_position_posix(int32_t *x, int32_t *y)
{
    SDL_GetGlobalMouseState(x, y);
    return true;
}

static void *shown_cursor;

static void *cursor_set_posix(void *cursor)
{
    void *before = shown_cursor;
    shown_cursor = cursor;
    return before;
}

static bool message_peek_posix(platform_message *msg, uint32_t first, uint32_t last, bool remove)
{
    return false;
}

static bool message_get_posix(platform_message *msg)
{
    msg->message = 0;
    return false;
}

static void message_wait_posix(void)
{
    SDL_Delay(5);
}

static bool message_post_posix(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return false;
}

static int32_t message_box_posix(void *window, const char *text, const char *caption, uint32_t type)
{
    uint32_t flags = (type & 0x10) ? SDL_MESSAGEBOX_ERROR : SDL_MESSAGEBOX_INFORMATION;
    SDL_ShowSimpleMessageBox(flags, caption ? caption : "", text ? text : "", (SDL_Window *) window);
    return PLATFORM_ANSWER_OK;
}

static platform_bindings bound = {
    time_ms_posix,
    time_ms_posix,
    key_state_posix,
    cursor_position_posix,
    cursor_set_posix,
    message_peek_posix,
    message_get_posix,
    message_wait_posix,
    message_post_posix,
    message_box_posix,
};

void platform_bind(const platform_bindings *bindings)
{
    bound.time_ms = bindings->time_ms ? bindings->time_ms : time_ms_posix;
    bound.tick_count = bindings->tick_count ? bindings->tick_count : time_ms_posix;
    bound.key_state = bindings->key_state ? bindings->key_state : key_state_posix;
    bound.cursor_position = bindings->cursor_position ? bindings->cursor_position : cursor_position_posix;
    bound.cursor_set = bindings->cursor_set ? bindings->cursor_set : cursor_set_posix;
    bound.message_peek = bindings->message_peek ? bindings->message_peek : message_peek_posix;
    bound.message_get = bindings->message_get ? bindings->message_get : message_get_posix;
    bound.message_wait = bindings->message_wait ? bindings->message_wait : message_wait_posix;
    bound.message_post = bindings->message_post ? bindings->message_post : message_post_posix;
    bound.message_box = bindings->message_box ? bindings->message_box : message_box_posix;
}

const platform_bindings *platform_bound(void)
{
    return &bound;
}

uint32_t platform_time_ms(void)
{
    return bound.time_ms();
}

uint32_t platform_tick_count(void)
{
    return bound.tick_count();
}

int16_t platform_key_state(int32_t key)
{
    return bound.key_state(key);
}

bool platform_cursor_position(int32_t *x, int32_t *y)
{
    return bound.cursor_position(x, y);
}

void *platform_cursor_set(void *cursor)
{
    return bound.cursor_set(cursor);
}

bool platform_message_peek(platform_message *msg, uint32_t first, uint32_t last, bool remove)
{
    return bound.message_peek(msg, first, last, remove);
}

bool platform_message_get(platform_message *msg)
{
    return bound.message_get(msg);
}

void platform_message_wait(void)
{
    bound.message_wait();
}

void platform_message_dispatch(const platform_message *msg)
{
}

bool platform_message_post(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return bound.message_post(window, message, wparam, lparam);
}

intptr_t platform_message_default(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return 0;
}

int32_t platform_message_box(void *window, const char *text, const char *caption, uint32_t type)
{
    return bound.message_box(window, text, caption, type);
}

int32_t platform_cursor_show(bool show)
{
    return SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE) == SDL_ENABLE ? 0 : -1;
}

void platform_message_post_quit(int32_t code)
{
    SDL_Event quit;
    memset(&quit, 0, sizeof(quit));
    quit.type = SDL_QUIT;
    SDL_PushEvent(&quit);
}

void platform_window_update(void *window)
{
}

void platform_window_set_focus(void *window)
{
}

void platform_window_redraw(void *window)
{
}

void platform_window_minimize(void *window)
{
    if (window) {
        SDL_MinimizeWindow((SDL_Window *) window);
    }
}

void platform_window_raise(void *window)
{
    if (window) {
        SDL_RaiseWindow((SDL_Window *) window);
    }
}

bool platform_window_is_foreground(void *window)
{
    return window && (SDL_GetWindowFlags((SDL_Window *) window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool platform_window_is_minimized(void *window)
{
    return window && (SDL_GetWindowFlags((SDL_Window *) window) & SDL_WINDOW_MINIMIZED) != 0;
}

void platform_window_validate(void *window)
{
}

void platform_window_screen_rect(void *window, platform_rect *rect)
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    if (window) {
        SDL_GetWindowPosition((SDL_Window *) window, &x, &y);
        SDL_GetWindowSize((SDL_Window *) window, &width, &height);
    }
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;
}

void platform_window_outer_size(void *window, int32_t width, int32_t height, int32_t *outer_width, int32_t *outer_height)
{
    *outer_width = width;
    *outer_height = height;
}

static bool window_is_fullscreen(SDL_Window *window)
{
    return (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
}

void platform_window_move(void *window, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (window && !window_is_fullscreen((SDL_Window *) window)) {
        SDL_SetWindowPosition((SDL_Window *) window, x, y);
        SDL_SetWindowSize((SDL_Window *) window, width, height);
    }
}

void platform_window_resize(void *window, int32_t width, int32_t height)
{
    if (window && !window_is_fullscreen((SDL_Window *) window)) {
        SDL_SetWindowSize((SDL_Window *) window, width, height);
    }
}

void *platform_desktop_text_begin(void)
{
    return NULL;
}

void platform_desktop_text_size(void *dc, const char *text, int32_t length, int32_t *width, int32_t *height)
{
    *width = 0;
    *height = 0;
}

void platform_desktop_text_draw(void *dc, int32_t x, int32_t y, const char *text, int32_t length, uint32_t background)
{
}

void platform_desktop_text_end(void *dc)
{
}

static void run_detached(const char *program, const char *argument)
{
    pid_t child = fork();
    if (child == 0) {
        if (fork() == 0) {
            execlp(program, program, argument, (char *) NULL);
        }
        _exit(0);
    }
    if (child > 0) {
        waitpid(child, NULL, 0);
    }
}

void platform_shell_open(void *window, const char *operation, const char *file)
{
#if defined(__ANDROID__)
    SDL_OpenURL(file);
#elif defined(__APPLE__)
    run_detached("open", file);
#else
    run_detached("xdg-open", file);
#endif
}

void platform_debug_output(const char *text)
{
    fputs(text, stderr);
}

static SDL_DisplayMode desktop_mode(void)
{
    SDL_DisplayMode mode;
    memset(&mode, 0, sizeof(mode));
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
    }
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
        mode.w = 1024;
        mode.h = 768;
        mode.format = SDL_PIXELFORMAT_RGB888;
    }
    return mode;
}

int32_t platform_desktop_width(void)
{
    return desktop_mode().w;
}

int32_t platform_desktop_height(void)
{
    return desktop_mode().h;
}

int32_t platform_desktop_color_depth(void)
{
    int32_t bits = (int32_t) SDL_BITSPERPIXEL(desktop_mode().format);
    return bits >= 24 ? 32 : bits;
}

void platform_desktop_palette(void *entries, int32_t count)
{
    memset(entries, 0, (size_t) count * 4u);
}

int32_t platform_memory_available(void)
{
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || page_size <= 0) {
        return INT32_MAX;
    }
    uint64_t bytes = (uint64_t) pages * (uint64_t) page_size;
    return bytes > INT32_MAX ? INT32_MAX : (int32_t) bytes;
}

bool platform_time_counter(uint64_t *count)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return false;
    }
    *count = (uint64_t) now.tv_sec * 1000000000u + (uint64_t) now.tv_nsec;
    return true;
}

bool platform_time_frequency(uint64_t *frequency)
{
    *frequency = 1000000000u;
    return true;
}

static int32_t web_links_allowed;

void platform_web_links_allow(int32_t allowed)
{
    web_links_allowed = allowed;
}

int32_t platform_web_links_allowed(void)
{
    return web_links_allowed;
}

void platform_url_open(const char *url)
{
    if (web_links_allowed) {
        platform_shell_open(0, "open", url);
    }
}
