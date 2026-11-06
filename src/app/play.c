// Play mode of tutankhamun: the game in an SDL window of its own. It does what a Windows message loop, window set-up
// and window procedure did around the game: runs the frames, presents the 16-bit frame buffer (platform/screen.c),
// feeds mouse and keyboard into the input globals, and pumps messages for the game's own modal loops. Sound is Miles
// on SDL_mixer (miles_sdl.c), video Bink on Akhenaten's decoder (video_bink.c). The display size and full screen
// state come from the command line or the [display] section of tutankhamun.ini, and are written back when changed.
//
// What the window does differently from a plain SDL program, so that the game behaves as it did on Windows:
// - the game stays active when the window loses the focus (WM_ACTIVATEAPP used to pause it);
// - only the game's two modal loops (the message box and the video loop) get the message pump; any other unfiltered
//   peek gets Windows' answer, and a filtered peek is always Windows' own;
// - in a video a key down or a left click ends it as the Win32 message would, and the key's text is not passed on;
// - the screen fade draws on the frame buffer in RGB565 when the game draws in 565 (display mode 2), else RGB555, and
//   takes as many random numbers as on the DirectDraw surface (mode 1: one per row).

#include "app/play.h"

#include "app/app_icon.h"
#include "app/media_lib.h"
#include "app/play_cursors.h"
#include "building/building_data.h"
#include "graphics/graphics_data.h"
#include "input/input_data.h"
#include "io/game_file.h"
#include "platform/platform_data.h"
#include "window/window_data.h"
#include "window/window_id.h"
#include "game/discord_presence.h"
#include "graphics/screen_fade.h"
#include "platform/platform_screen.h"
#include "platform/platform_dialog.h"
#include "platform/platform_display.h"
#include "platform/platform_shell.h"
#include "platform/platform_win32.h"
#include "platform/screen.h"
#include "video/bink.h"
#include "window/display_resolution_list.h"
#include "window/free_play.h"
#include "window/window_proc.h"

#include <windows.h>
#include <mmsystem.h>
#include <intrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_syswm.h"


static void no_log(const char *format, ...)
{
    (void) format;
}

void (*play_log)(const char *format, ...) = no_log;
static play_host host;


typedef LRESULT(CALLBACK *window_proc_fn)(HWND, UINT, WPARAM, LPARAM);

static struct {
    int32_t cursor_x;
    int32_t cursor_y;
    int32_t frame;
    int32_t quit;
    int32_t running;
    int32_t modal_frames;
    uint32_t video_poll_ms;
    int32_t has_message;
    MSG message;
    int32_t other_peeks;
    uint8_t key_down[256];
    uint8_t key_pressed[256];
} play;

static bool play_cursor_position(int32_t *x, int32_t *y)
{
    *x = play.cursor_x;
    *y = play.cursor_y;
    return true;
}

static void *shown_cursor;

static void *play_cursor_set(void *cursor)
{
    void *before = shown_cursor;
    if (cursor != shown_cursor) {
        play_cursor_show(cursor);
        shown_cursor = cursor;
    }
    return before;
}

static int16_t play_key_state(int32_t vk)
{
    if (vk < 0 || vk > 255) {
        return 0;
    }
    int16_t state = (int16_t) ((play.key_down[vk] ? 0x8000 : 0) | play.key_pressed[vk]);
    play.key_pressed[vk] = 0;
    return state;
}

static void key_state(int32_t vk, int32_t down)
{
    play.key_pressed[vk] |= (uint8_t) (down && !play.key_down[vk]);
    play.key_down[vk] = (uint8_t) down;
}

static void modal_pump(void);

static void modal_message(UINT message, WPARAM wparam)
{
    memset(&play.message, 0, sizeof(play.message));
    play.message.message = message;
    play.message.wParam = wparam;
    play.message.time = GetTickCount();
    play.has_message = 1;
}

static BOOL take_modal_message(MSG *msg, UINT remove)
{
    if (!play.has_message) {
        return FALSE;
    }
    *msg = play.message;
    if (remove & PM_REMOVE) {
        play.has_message = 0;
    }
    return TRUE;
}


static int32_t in_modal_loop(void)
{
    return window_current_id == WINDOW_VIDEO || window_current_id == WINDOW_MESSAGE_BOX;
}

static BOOL peek_message(MSG *msg, UINT first, UINT last, UINT remove, void *caller)
{
    if (first || last || !play.running) {
        return PeekMessageA(msg, 0, first, last, remove);
    }
    if (!in_modal_loop()) {
        if (!play.other_peeks++) {
            play_log("play: frame %d unfiltered PeekMessageA from 0x%08X in window %d: no pump outside the modal loops", play.frame,
                (uint32_t) (uintptr_t) caller, window_current_id);
        }
        return PeekMessageA(msg, 0, first, last, remove);
    }
    if (take_modal_message(msg, remove)) {
        return TRUE;
    }
    if (PeekMessageA(msg, 0, 0, 0, remove)) {
        return TRUE;
    }
    modal_pump();
    return take_modal_message(msg, remove);
}

static bool play_message_peek(platform_message *msg, uint32_t first, uint32_t last, bool remove)
{
    MSG *native = (MSG *) msg->native;
    if (!peek_message(native, first, last, remove ? PM_REMOVE : PM_NOREMOVE, _ReturnAddress())) {
        return false;
    }
    msg->message = native->message;
    return true;
}

static bool play_message_get(platform_message *msg)
{
    MSG *native = (MSG *) msg->native;
    bool more;
    if (play.running && take_modal_message(native, PM_REMOVE)) {
        more = native->message != WM_QUIT;
    } else {
        more = GetMessageA(native, 0, 0, 0) != 0;
    }
    msg->message = native->message;
    return more;
}

static void play_message_wait(void)
{
    if (!play.running) {
        WaitMessage();
        return;
    }
    SDL_Delay(5);
}

static int32_t default_box_answer(uint32_t type)
{
    switch (type & MB_TYPEMASK) {
        case MB_ABORTRETRYIGNORE:
            return IDIGNORE;
        case MB_YESNO:
        case MB_YESNOCANCEL:
            return IDNO;
        case MB_RETRYCANCEL:
        case MB_OKCANCEL:
            return IDCANCEL;
        default:
            return IDOK;
    }
}

static int32_t play_message_box(void *window, const char *text, const char *caption, uint32_t type)
{
    (void) window;
    static const struct {
        int32_t count;
        int32_t ids[3];
        const char *texts[3];
    } layouts[] = {
        { 1, { IDOK }, { "OK" } },
        { 2, { IDOK, IDCANCEL }, { "OK", "Cancel" } },
        { 3, { IDABORT, IDRETRY, IDIGNORE }, { "Abort", "Retry", "Ignore" } },
        { 3, { IDYES, IDNO, IDCANCEL }, { "Yes", "No", "Cancel" } },
        { 2, { IDYES, IDNO }, { "Yes", "No" } },
        { 2, { IDRETRY, IDCANCEL }, { "Retry", "Cancel" } },
    };
    UINT layout = type & MB_TYPEMASK;
    if (layout >= sizeof(layouts) / sizeof(layouts[0])) {
        layout = 0;
    }
    SDL_MessageBoxButtonData buttons[3];
    int32_t count = layouts[layout].count;
    for (int32_t i = 0; i < count; i++) {
        buttons[i].flags = (i == 0 ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT : 0) | (i == count - 1 ? SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0);
        buttons[i].buttonid = layouts[layout].ids[i];
        buttons[i].text = layouts[layout].texts[i];
    }
    UINT icon = type & MB_ICONMASK;
    SDL_MessageBoxData box;
    memset(&box, 0, sizeof(box));
    box.flags = icon == MB_ICONERROR                        ? SDL_MESSAGEBOX_ERROR
        : icon == MB_ICONWARNING || icon == MB_ICONQUESTION ? SDL_MESSAGEBOX_WARNING
                                                            : SDL_MESSAGEBOX_INFORMATION;
    box.window = (SDL_Window *) platform_screen_window();
    box.title = caption ? caption : "";
    box.message = text ? text : "";
    box.numbuttons = count;
    box.buttons = buttons;
    int32_t answer = -1;
    if (SDL_ShowMessageBox(&box, &answer) != 0) {
        play_log("play: message box \"%s\" not shown: %s", box.title, SDL_GetError());
        answer = default_box_answer(type);
    } else if (answer < 0) {
        answer = layouts[layout].ids[count - 1];
    }
    play_log("play: frame %d message box \"%s\": %s: answer %d", play.frame, box.title, box.message, answer);
    return answer;
}

enum play_posted_keys {
    POSTED_KEYS_MAX = 16,
};

static struct {
    UINT message;
    WPARAM wparam;
    LPARAM lparam;
} posted_keys[POSTED_KEYS_MAX];
static int32_t posted_key_count;

static int32_t is_key_message(UINT message)
{
    return message == WM_KEYDOWN || message == WM_KEYUP || message == WM_CHAR || message == WM_SYSKEYDOWN || message == WM_SYSKEYUP;
}

static bool play_message_post(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    if (!play.running || window != platform_main_window || !is_key_message(message)) {
        return PostMessageA((HWND) window, message, wparam, lparam) != 0;
    }
    if (posted_key_count >= POSTED_KEYS_MAX) {
        play_log("play: frame %d posted key 0x%X dropped, the queue is full", play.frame, (uint32_t) wparam);
        return false;
    }
    posted_keys[posted_key_count].message = message;
    posted_keys[posted_key_count].wparam = wparam;
    posted_keys[posted_key_count].lparam = lparam;
    posted_key_count++;
    return true;
}

static uint32_t play_tick_count(void)
{
    return timeGetTime();
}

static const platform_bindings play_bindings = {
    .tick_count = play_tick_count,
    .key_state = play_key_state,
    .cursor_position = play_cursor_position,
    .cursor_set = play_cursor_set,
    .message_peek = play_message_peek,
    .message_get = play_message_get,
    .message_wait = play_message_wait,
    .message_post = play_message_post,
    .message_box = play_message_box,
};

void play_bind(const play_host *h)
{
    host = *h;
    play_log = host.log;
    platform_bind(&play_bindings);
    media_sound = miles_sdl_library;
    media_video = play_video_library;
}

static void mouse_message(UINT message, int32_t x, int32_t y)
{
    play.cursor_x = x;
    play.cursor_y = y;
    switch (message) {
        case WM_LBUTTONDOWN:
            mouse_left_clicked = 1;
            mouse_buttons |= 1;
            break;
        case WM_LBUTTONUP:
            mouse_buttons &= (uint8_t) ~1;
            break;
        case WM_RBUTTONDOWN:
            mouse_buttons |= 2;
            break;
        case WM_RBUTTONUP:
            mouse_buttons &= (uint8_t) ~2;
            break;
        default:
            break;
    }
    mouse_window_x = (int16_t) x;
    mouse_window_y = (int16_t) y;
}

static void window_message(UINT message, WPARAM wparam, LPARAM lparam)
{
    ((window_proc_fn) window_proc)(platform_main_window, message, wparam, lparam);
}

static int32_t virtual_key(SDL_Keycode key)
{
    if (key >= SDLK_a && key <= SDLK_z) {
        return 'A' + (key - SDLK_a);
    }
    if (key >= SDLK_0 && key <= SDLK_9) {
        return key;
    }
    if (key >= SDLK_F1 && key <= SDLK_F12) {
        return VK_F1 + (key - SDLK_F1);
    }
    if (key >= SDLK_KP_1 && key <= SDLK_KP_9) {
        return VK_NUMPAD1 + (key - SDLK_KP_1);
    }
    switch (key) {
        case SDLK_KP_0:
            return VK_NUMPAD0;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return VK_RETURN;
        case SDLK_ESCAPE:
            return VK_ESCAPE;
        case SDLK_BACKSPACE:
            return VK_BACK;
        case SDLK_TAB:
            return VK_TAB;
        case SDLK_SPACE:
            return VK_SPACE;
        case SDLK_LEFT:
            return VK_LEFT;
        case SDLK_RIGHT:
            return VK_RIGHT;
        case SDLK_UP:
            return VK_UP;
        case SDLK_DOWN:
            return VK_DOWN;
        case SDLK_PAGEUP:
            return VK_PRIOR;
        case SDLK_PAGEDOWN:
            return VK_NEXT;
        case SDLK_HOME:
            return VK_HOME;
        case SDLK_END:
            return VK_END;
        case SDLK_INSERT:
            return VK_INSERT;
        case SDLK_DELETE:
            return VK_DELETE;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            return VK_SHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            return VK_CONTROL;
        case SDLK_LALT:
        case SDLK_RALT:
            return VK_MENU;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            return VK_OEM_MINUS;
        case SDLK_EQUALS:
        case SDLK_KP_PLUS:
            return VK_OEM_PLUS;
        case SDLK_COMMA:
            return VK_OEM_COMMA;
        case SDLK_PERIOD:
            return VK_OEM_PERIOD;
        case SDLK_LEFTBRACKET:
            return VK_OEM_4;
        case SDLK_RIGHTBRACKET:
            return VK_OEM_6;
        case SDLK_PAUSE:
            return VK_PAUSE;
        default:
            return 0;
    }
}

static void key_event(const SDL_KeyboardEvent *e)
{
    int32_t vk = virtual_key(e->keysym.sym);
    if (!vk) {
        return;
    }
    int32_t down = e->type == SDL_KEYDOWN;
    key_state(vk, down);
    if (vk == VK_MENU || vk == VK_SHIFT || vk == VK_CONTROL) {
        key_state(vk == VK_MENU ? VK_LMENU : vk == VK_SHIFT ? VK_LSHIFT : VK_LCONTROL, down);
    }
    int32_t alt = (e->keysym.mod & KMOD_ALT) != 0 && vk != VK_MENU;
    LPARAM lparam = 1 | (LPARAM) (MapVirtualKeyA((UINT) vk, 0) << 16) | (alt ? 1 << 29 : 0);
    if (e->type == SDL_KEYDOWN) {
        window_message(alt ? WM_SYSKEYDOWN : WM_KEYDOWN, (WPARAM) vk, lparam | (e->repeat ? 1 << 30 : 0));
        if (!alt && (vk == VK_BACK || vk == VK_TAB || vk == VK_RETURN || vk == VK_ESCAPE)) {
            window_message(WM_CHAR, (WPARAM) vk, lparam);
        }
    } else {
        window_message(alt ? WM_SYSKEYUP : WM_KEYUP, (WPARAM) vk, lparam | (LPARAM) 0xC0000000u);
    }
}

static void text_event(const char *text)
{
    const uint8_t *p = (const uint8_t *) text;
    while (*p) {
        uint32_t c = *p++;
        if (c >= 0xC0 && c < 0xE0 && (*p & 0xC0) == 0x80) {
            c = ((c & 0x1F) << 6) | (*p++ & 0x3F);
        } else if (c >= 0x80) {
            continue;
        }
        if (c < 0x100) {
            window_message(WM_CHAR, (WPARAM) c, 1);
        }
    }
}

static void handle_event(const SDL_Event *e)
{
    switch (e->type) {
        case SDL_QUIT:
            play.quit = 1;
            break;
        case SDL_WINDOWEVENT:
            if (e->window.event == SDL_WINDOWEVENT_EXPOSED || e->window.event == SDL_WINDOWEVENT_RESTORED) {
                window_redraw_request = 1;
                g_force_refresh = 1;
            }
            if (e->window.event == SDL_WINDOWEVENT_FOCUS_LOST || e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                int32_t lost = e->window.event == SDL_WINDOWEVENT_FOCUS_LOST;
                play_log("play: frame %d focus %s (window %d)", play.frame, lost ? "lost" : "gained", window_current_id);
            }
            break;
        case SDL_MOUSEMOTION:
            mouse_message(WM_MOUSEMOVE, e->motion.x, e->motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int32_t down = e->type == SDL_MOUSEBUTTONDOWN;
            if (e->button.button == SDL_BUTTON_LEFT || e->button.button == SDL_BUTTON_RIGHT) {
                key_state(e->button.button == SDL_BUTTON_LEFT ? VK_LBUTTON : VK_RBUTTON, down);
            }
            if (e->button.button == SDL_BUTTON_LEFT) {
                mouse_message(down ? WM_LBUTTONDOWN : WM_LBUTTONUP, e->button.x, e->button.y);
            } else if (e->button.button == SDL_BUTTON_RIGHT) {
                mouse_message(down ? WM_RBUTTONDOWN : WM_RBUTTONUP, e->button.x, e->button.y);
            }
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            key_event(&e->key);
            break;
        case SDL_TEXTINPUT:
            text_event(e->text.text);
            break;
        default:
            break;
    }
}

void play_load_save(const char *path)
{
    if (game_file_load_saved_game(path) != 0) {
        play_log("play: cannot load %s", path);
        return;
    }
    window_current_id = WINDOW_CITY;
    window_redraw_request = 1;
    play_log("play: loaded %s", path);
}

static HWND window_handle(SDL_Window *window)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    return window && SDL_GetWindowWMInfo(window, &info) ? info.info.win.window : 0;
}

static size_t frame_buffer_bytes;
static int32_t fullscreen_setting;
static int32_t fullscreen;

static void display_init(void)
{
    size_t bytes = (size_t) g_screen_width * (size_t) g_screen_height * 2;
    void **pixels = (void **) &screen_rgb565_ptr;
    if (bytes != frame_buffer_bytes || !*pixels) {
        free(*pixels);
        *pixels = calloc(1, bytes);
        frame_buffer_bytes = bytes;
    }
    SDL_Window *window = platform_screen_window();
    if (window) {
        if (settings_data.fullscreen != fullscreen_setting) {
            fullscreen_setting = settings_data.fullscreen;
            fullscreen = !fullscreen;
            SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }
        if (!fullscreen) {
            SDL_SetWindowSize(window, g_screen_width, g_screen_height);
        }
    }
    SetRect((RECT *) &window_screen_rect, 0, 0, g_screen_width, g_screen_height);
    desktop_width = g_screen_width;
    desktop_height = g_screen_height;
    screen_fade_set_frame(&fade_object, screen_rgb565_ptr, g_screen_width, g_screen_height);
    play_log("play: display %dx%d%s", g_screen_width, g_screen_height, fullscreen ? ", full screen" : "");
}

static int32_t display_shutdown(int32_t release_display)
{
    (void) release_display;
    int32_t result = dd_enabled;
    if (result) {
        result = 1;
        dd_enabled = 0;
    }
    return result;
}

static int32_t display_begin(void)
{
    g_skip_refresh = 1;
    display_shutdown(0);
    display_init();
    dd_good_display_mode = settings_data.display_mode;
    dd_good_fullscreen = settings_data.fullscreen;
    dd_mode_retry_flag = 0;
    dd_enabled = 1;
    memset(screen_rgb565_ptr, 0, frame_buffer_bytes);
    g_skip_refresh = 0;
    app_active = 1;
    window_redraw_request = 1;
    return 1;
}

static void deliver_posted_keys(void)
{
    int32_t count = posted_key_count;
    posted_key_count = 0;
    for (int32_t i = 0; i < count; i++) {
        window_message(posted_keys[i].message, posted_keys[i].wparam, posted_keys[i].lparam);
    }
}

static void frame_begin(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        handle_event(&e);
    }
    deliver_posted_keys();
}

static void frame_end(void)
{
    platform_screen_update();
    platform_screen_render();
    if (game_quit_request == 1) {
        play.quit = 1;
    }
    play.frame++;
}

static void video_pump(void)
{
    if (play.quit) {
        if (!play.has_message) {
            play_log("play: frame %d quit asked in a video: the video ends", play.frame);
            modal_message(WM_KEYDOWN, VK_ESCAPE);
        }
        return;
    }
    uint32_t now = SDL_GetTicks();
    if (now - play.video_poll_ms < 10) {
        return;
    }
    play.video_poll_ms = now;
    SDL_Event e;
    while (!play.has_message && SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            play.quit = 1;
            play_log("play: frame %d window closed in a video: the video ends", play.frame);
            modal_message(WM_KEYDOWN, VK_ESCAPE);
        } else if (e.type == SDL_KEYDOWN) {
            int32_t vk = virtual_key(e.key.keysym.sym);
            play_log("play: frame %d key %d in a video: the video ends", play.frame, vk);
            modal_message(WM_KEYDOWN, (WPARAM) vk);
        } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            play_log("play: frame %d click in a video: the video ends", play.frame);
            mouse_message(WM_MOUSEMOVE, e.button.x, e.button.y);
            modal_message(WM_LBUTTONDOWN, 0);
        } else if (e.type == SDL_KEYUP) {
            int32_t vk = virtual_key(e.key.keysym.sym);
            if (vk) {
                key_state(vk, 0);
            }
        } else if (e.type != SDL_TEXTINPUT) {
            handle_event(&e);
        }
    }
}

static void modal_pump(void)
{
    if (window_current_id == WINDOW_VIDEO) {
        video_pump();
        return;
    }
    frame_end();
    play.modal_frames++;
    frame_begin();
    if (play.quit && !play.has_message) {
        play_log("play: frame %d quit in the modal loop of window %d: WM_QUIT", play.frame, window_current_id);
        modal_message(WM_QUIT, 0);
    }
}

static int32_t fullscreen_setting_before_video;

int32_t play_fullscreen_for_video(void)
{
    fullscreen_setting_before_video = fullscreen_setting;
    fullscreen_setting = fullscreen;
    return fullscreen;
}

void play_fullscreen_after_video(void)
{
    fullscreen_setting = fullscreen_setting_before_video;
}

static void install_icon_and_cursors(void)
{
    SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom((void *) app_icon_argb, APP_ICON_SIZE, APP_ICON_SIZE, 32,
        APP_ICON_SIZE * (int) sizeof(app_icon_argb[0]), SDL_PIXELFORMAT_ARGB8888);
    if (icon) {
        SDL_SetWindowIcon(platform_screen_window(), icon);
        SDL_FreeSurface(icon);
    }
    int32_t cursors = play_cursors_install();
    play_log("play: icon %s, %d own cursors made", icon ? "set" : "missing", cursors);
}

enum play_display_file {
    DISPLAY_FILE_VALUE_SIZE = 16,
    DISPLAY_MODES_MAX = 128,
};

static const char display_ini_section[] = "display";

static struct {
    int32_t chosen;
    int32_t width;
    int32_t height;
    int32_t fullscreen;
} display_start;

static void display_ini_path(char *path, size_t size)
{
    DWORD length = GetModuleFileNameA(0, path, (DWORD) size);
    char *slash = length > 0 && length < size ? strrchr(path, '\\') : 0;
    char *dot = slash ? strrchr(slash, '.') : 0;
    if (!dot || (size_t) (dot - path) + sizeof(".ini") > size) {
        path[0] = 0;
        return;
    }
    memcpy(dot, ".ini", sizeof(".ini"));
}

static play_display_request display_file_read(void)
{
    play_display_request remembered = { 0, 0, -1 };
    char path[MAX_PATH];
    display_ini_path(path, sizeof(path));
    if (!path[0]) {
        return remembered;
    }
    remembered.width = (int32_t) GetPrivateProfileIntA(display_ini_section, "width", 0, path);
    remembered.height = (int32_t) GetPrivateProfileIntA(display_ini_section, "height", 0, path);
    remembered.fullscreen = (int32_t) GetPrivateProfileIntA(display_ini_section, "fullscreen", -1, path);
    if (remembered.fullscreen > 0) {
        remembered.fullscreen = 1;
    }
    return remembered;
}

static int32_t display_ini_write(const char *path, const char *key, int32_t value)
{
    char text[DISPLAY_FILE_VALUE_SIZE];
    snprintf(text, sizeof(text), "%d", value);
    return WritePrivateProfileStringA(display_ini_section, key, text, path) != 0;
}

static void display_file_write(void)
{
    char path[MAX_PATH];
    display_ini_path(path, sizeof(path));
    int32_t written = path[0] && display_ini_write(path, "width", g_screen_width) && display_ini_write(path, "height", g_screen_height) &&
        display_ini_write(path, "fullscreen", fullscreen);
    play_log(written ? "play: display %dx%d%s remembered in %s" : "play: cannot remember the display %dx%d%s in %s", g_screen_width,
        g_screen_height, fullscreen ? ", full screen" : "", path[0] ? path : "?");
}

static void display_limit_from_sdl(void)
{
    SDL_DisplayMode desktop;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0 && SDL_GetDesktopDisplayMode(0, &desktop) == 0) {
        display_resolution_set_limit(desktop.w, desktop.h);
        play_log("play: the display is %dx%d: sizes from 640x480 up to it", desktop.w, desktop.h);
    }
}

int32_t play_display_prepare(const play_display_request *command_line)
{
    int32_t use_ini = 1;
    display_limit_from_sdl();
    play_display_request none = { 0, 0, -1 };
    play_display_request request = use_ini ? display_file_read() : none;
    if (command_line->width > 0 && command_line->height > 0) {
        request.width = command_line->width;
        request.height = command_line->height;
    }
    if (command_line->fullscreen >= 0) {
        request.fullscreen = command_line->fullscreen;
    }
    display_start.chosen = use_ini;
    display_start.fullscreen = request.fullscreen;
    if (request.width <= 0 || request.height <= 0) {
        return 0;
    }
    screen_size size = display_resolution_nearest(request.width, request.height);
    if (size.width != request.width || size.height != request.height) {
        fprintf(stderr, "the size %dx%d is not supported (640x480 up to the display's size): %dx%d is used\n", request.width, request.height,
            size.width, size.height);
        play_log("play: size %dx%d not supported, the nearest %dx%d is used", request.width, request.height, size.width, size.height);
    }
    display_resolution_start_custom(size);
    return display_resolution_layout_mode(size.width, size.height);
}

static void display_list_from_sdl(SDL_Window *window)
{
    screen_size modes[DISPLAY_MODES_MAX];
    int32_t count = 0;
    int32_t display = window ? SDL_GetWindowDisplayIndex(window) : -1;
    int32_t available = display >= 0 ? SDL_GetNumDisplayModes(display) : 0;
    for (int32_t i = 0; i < available && count < DISPLAY_MODES_MAX; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(display, i, &mode) == 0) {
            modes[count].width = mode.w;
            modes[count].height = mode.h;
            count++;
        }
    }
    display_resolution_list_set_display_modes(modes, count);
    display_resolution_list_allow(1);
    play_log("play: %d display modes, %d resolutions in the display options", available, display_resolution_list_count());
}

static void display_start_fullscreen(SDL_Window *window)
{
    if (display_start.fullscreen < 0) {
        return;
    }
    settings_data.fullscreen = display_start.fullscreen;
    fullscreen_setting = settings_data.fullscreen;
    fullscreen = settings_data.fullscreen;
    if (window && fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void display_start_keep(void)
{
    display_start.width = g_screen_width;
    display_start.height = g_screen_height;
    display_start.fullscreen = fullscreen;
}

static void display_remember_if_changed(void)
{
    if (g_screen_width != display_start.width || g_screen_height != display_start.height || fullscreen != display_start.fullscreen) {
        display_file_write();
    }
}

int32_t play_run(const char *save)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    if (!platform_screen_create("Tutankhamun", g_screen_width, g_screen_height)) {
        fprintf(stderr, "cannot open the window: %s\n", SDL_GetError());
        return 2;
    }
    platform_main_window = window_handle(platform_screen_window());
    install_icon_and_cursors();
    app_active = 1;
    SetRect((RECT *) &window_screen_rect, 0, 0, g_screen_width, g_screen_height);
    desktop_width = g_screen_width;
    desktop_height = g_screen_height;
    frame_buffer_bytes = (size_t) g_screen_width * (size_t) g_screen_height * 2;
    fullscreen_setting = settings_data.fullscreen;
    screen_fade_set_frame(&fade_object, screen_rgb565_ptr, g_screen_width, g_screen_height);
    dd_enabled = 1;
    platform_display_set_host(display_begin, display_shutdown);
    platform_web_links_allow(1);
    free_play_allow(1);
    discord_presence_start();
    if (display_start.chosen) {
        display_list_from_sdl(platform_screen_window());
        display_start_fullscreen(platform_screen_window());
        display_start_keep();
    }
    if (!video_player_ptr) {
        video_player_ptr = video_bink_init(calloc(1, sizeof(video_player)), platform_main_window);
    }
    play_log("play: window %dx%d, hwnd %p", g_screen_width, g_screen_height, (void *) platform_main_window);
    play.running = 1;
    video_play_intro();
    if (save) {
        play_load_save(save);
    }
    SDL_StartTextInput();
    while (!play.quit) {
        frame_begin();
        host.run_tick();
        host.run_frame();
        frame_end();
    }
    if (display_start.chosen) {
        display_remember_if_changed();
    }
    play_log("play: quit after %d frames (%d in the game's message boxes, %d unfiltered peeks outside them)", play.frame, play.modal_frames,
        play.other_peeks);
    discord_presence_stop();
    platform_screen_destroy();
    SDL_Quit();
    return 0;
}
