// The platform functions on Windows. The clocks, the keyboard and cursor state, the message pump and the message box
// call the function the front end bound to them (platform_bind), or else the Windows function; the rest call Windows
// directly. A web address of the main menu's links opens in the desktop's browser only when the play mode allowed the
// links.

#include "platform/platform_win32.h"

#include "platform/platform_dialog.h"
#include "platform/platform_file.h"
#include "platform/platform_input.h"
#include "platform/platform_message.h"
#include "platform/platform_shell.h"
#include "platform/platform_system.h"
#include "platform/platform_time.h"
#include "platform/platform_window.h"

#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <assert.h>
#include <string.h>

static_assert(sizeof(platform_rect) == sizeof(RECT), "platform_rect is a Windows RECT");
static_assert(sizeof(MSG) <= sizeof(((platform_message *) 0)->native), "platform_message has room for a Windows MSG");

static_assert(PLATFORM_MSG_DESTROY == WM_DESTROY, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_MOVE == WM_MOVE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_SIZE == WM_SIZE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_ACTIVATE == WM_ACTIVATE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_PAINT == WM_PAINT, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_CLOSE == WM_CLOSE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_ACTIVATE_APP == WM_ACTIVATEAPP, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_SET_CURSOR == WM_SETCURSOR, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_DISPLAY_CHANGE == WM_DISPLAYCHANGE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_KEY_DOWN == WM_KEYDOWN, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_KEY_UP == WM_KEYUP, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_CHAR == WM_CHAR, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_SYS_KEY_DOWN == WM_SYSKEYDOWN, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_MOUSE_MOVE == WM_MOUSEMOVE, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_LEFT_DOWN == WM_LBUTTONDOWN, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_LEFT_UP == WM_LBUTTONUP, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_LEFT_DOUBLE_CLICK == WM_LBUTTONDBLCLK, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_RIGHT_DOWN == WM_RBUTTONDOWN, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_RIGHT_UP == WM_RBUTTONUP, "platform message ids are the Windows ones");
static_assert(PLATFORM_MSG_RIGHT_DOUBLE_CLICK == WM_RBUTTONDBLCLK, "platform message ids are the Windows ones");
static_assert(PLATFORM_KEY_BACKSPACE == VK_BACK, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_RETURN == VK_RETURN, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_SHIFT == VK_SHIFT, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_CONTROL == VK_CONTROL, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_ALT == VK_MENU, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_ESCAPE == VK_ESCAPE, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_PAGE_UP == VK_PRIOR, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_PAGE_DOWN == VK_NEXT, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_END == VK_END, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_HOME == VK_HOME, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_LEFT == VK_LEFT, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_UP == VK_UP, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_RIGHT == VK_RIGHT, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_DOWN == VK_DOWN, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_INSERT == VK_INSERT, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_DELETE == VK_DELETE, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_NUMPAD_1 == VK_NUMPAD1, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_NUMPAD_2 == VK_NUMPAD2, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_NUMPAD_3 == VK_NUMPAD3, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_NUMPAD_7 == VK_NUMPAD7, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_NUMPAD_8 == VK_NUMPAD8, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_MULTIPLY == VK_MULTIPLY, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_ADD == VK_ADD, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F1 == VK_F1, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F2 == VK_F2, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F3 == VK_F3, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F4 == VK_F4, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F5 == VK_F5, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F6 == VK_F6, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F7 == VK_F7, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F8 == VK_F8, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F9 == VK_F9, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F10 == VK_F10, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F11 == VK_F11, "platform key codes are the Windows virtual keys");
static_assert(PLATFORM_KEY_F12 == VK_F12, "platform key codes are the Windows virtual keys");

static uint32_t time_ms_windows(void)
{
    return timeGetTime();
}

static uint32_t tick_count_windows(void)
{
    return GetTickCount();
}

static int16_t key_state_windows(int32_t key)
{
    return GetAsyncKeyState(key);
}

static bool cursor_position_windows(int32_t *x, int32_t *y)
{
    POINT point;
    if (!GetCursorPos(&point)) {
        return false;
    }
    *x = point.x;
    *y = point.y;
    return true;
}

static void *cursor_set_windows(void *cursor)
{
    return SetCursor((HCURSOR) cursor);
}

static bool message_peek_windows(platform_message *msg, uint32_t first, uint32_t last, bool remove)
{
    MSG *native = (MSG *) msg->native;
    if (!PeekMessageA(native, 0, first, last, remove ? PM_REMOVE : PM_NOREMOVE)) {
        return false;
    }
    msg->message = native->message;
    return true;
}

static bool message_get_windows(platform_message *msg)
{
    MSG *native = (MSG *) msg->native;
    bool more = GetMessageA(native, 0, 0, 0) != 0;
    msg->message = native->message;
    return more;
}

static void message_wait_windows(void)
{
    WaitMessage();
}

static bool message_post_windows(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return PostMessageA((HWND) window, message, wparam, lparam) != 0;
}

static int32_t message_box_windows(void *window, const char *text, const char *caption, uint32_t type)
{
    return MessageBoxA((HWND) window, text, caption, type);
}

static platform_bindings bound = {
    time_ms_windows,
    tick_count_windows,
    key_state_windows,
    cursor_position_windows,
    cursor_set_windows,
    message_peek_windows,
    message_get_windows,
    message_wait_windows,
    message_post_windows,
    message_box_windows,
};

void platform_bind(const platform_bindings *bindings)
{
    bound.time_ms = bindings->time_ms ? bindings->time_ms : time_ms_windows;
    bound.tick_count = bindings->tick_count ? bindings->tick_count : tick_count_windows;
    bound.key_state = bindings->key_state ? bindings->key_state : key_state_windows;
    bound.cursor_position = bindings->cursor_position ? bindings->cursor_position : cursor_position_windows;
    bound.cursor_set = bindings->cursor_set ? bindings->cursor_set : cursor_set_windows;
    bound.message_peek = bindings->message_peek ? bindings->message_peek : message_peek_windows;
    bound.message_get = bindings->message_get ? bindings->message_get : message_get_windows;
    bound.message_wait = bindings->message_wait ? bindings->message_wait : message_wait_windows;
    bound.message_post = bindings->message_post ? bindings->message_post : message_post_windows;
    bound.message_box = bindings->message_box ? bindings->message_box : message_box_windows;
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
    const MSG *native = (const MSG *) msg->native;
    TranslateMessage(native);
    DispatchMessageA(native);
}

bool platform_message_post(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return bound.message_post(window, message, wparam, lparam);
}

intptr_t platform_message_default(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam)
{
    return DefWindowProcA((HWND) window, message, wparam, lparam);
}

int32_t platform_message_box(void *window, const char *text, const char *caption, uint32_t type)
{
    return bound.message_box(window, text, caption, type);
}

int32_t platform_cursor_show(bool show)
{
    return ShowCursor(show);
}

void platform_message_post_quit(int32_t code)
{
    PostQuitMessage(code);
}

void platform_window_update(void *window)
{
    UpdateWindow((HWND) window);
}

void platform_window_set_focus(void *window)
{
    SetFocus((HWND) window);
}

void platform_window_redraw(void *window)
{
    InvalidateRect((HWND) window, 0, TRUE);
}

void platform_window_minimize(void *window)
{
    ShowWindow((HWND) window, SW_MINIMIZE);
}

void platform_window_raise(void *window)
{
    SetActiveWindow(0);
    SetActiveWindow((HWND) window);
    SetForegroundWindow((HWND) window);
    ShowWindow((HWND) window, SW_SHOWMAXIMIZED);
}

bool platform_window_is_foreground(void *window)
{
    return GetForegroundWindow() == (HWND) window;
}

bool platform_window_is_minimized(void *window)
{
    return IsIconic((HWND) window) != 0;
}

void platform_window_validate(void *window)
{
    PAINTSTRUCT paint;
    BeginPaint((HWND) window, &paint);
    EndPaint((HWND) window, &paint);
}

void platform_window_screen_rect(void *window, platform_rect *rect)
{
    GetClientRect((HWND) window, (RECT *) rect);
    ClientToScreen((HWND) window, (POINT *) &rect->left);
    ClientToScreen((HWND) window, (POINT *) &rect->right);
}

void platform_window_outer_size(void *window, int32_t width, int32_t height, int32_t *outer_width, int32_t *outer_height)
{
    RECT rect;
    SetRect(&rect, 0, 0, width, height);
    DWORD ex_style = (DWORD) GetWindowLongA((HWND) window, GWL_EXSTYLE);
    BOOL menu = GetMenu((HWND) window) != 0;
    DWORD style = (DWORD) GetWindowLongA((HWND) window, GWL_STYLE);
    AdjustWindowRectEx(&rect, style, menu, ex_style);
    *outer_width = rect.right - rect.left;
    *outer_height = rect.bottom - rect.top;
}

void platform_window_move(void *window, int32_t x, int32_t y, int32_t width, int32_t height)
{
    SetWindowPos((HWND) window, 0, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void platform_window_resize(void *window, int32_t width, int32_t height)
{
    SetWindowPos((HWND) window, 0, 0, 0, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
    SetWindowPos((HWND) window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

void *platform_desktop_text_begin(void)
{
    return GetDC(0);
}

void platform_desktop_text_size(void *dc, const char *text, int32_t length, int32_t *width, int32_t *height)
{
    SIZE size;
    GetTextExtentPointA((HDC) dc, text, length, &size);
    *width = size.cx;
    *height = size.cy;
}

void platform_desktop_text_draw(void *dc, int32_t x, int32_t y, const char *text, int32_t length, uint32_t background)
{
    SetBkColor((HDC) dc, background);
    TextOutA((HDC) dc, x, y, text, length);
}

void platform_desktop_text_end(void *dc)
{
    ReleaseDC(0, (HDC) dc);
}

void platform_shell_open(void *window, const char *operation, const char *file)
{
    ShellExecuteA((HWND) window, operation, file, 0, 0, SW_SHOW);
}

void platform_debug_output(const char *text)
{
    OutputDebugStringA(text);
}

bool platform_dir_create(const char *path)
{
    return CreateDirectoryA(path, 0) != 0;
}

bool platform_dir_remove(const char *path)
{
    return RemoveDirectoryA(path) != 0;
}

static_assert(sizeof(WIN32_FIND_DATAA) <= sizeof(((platform_dir_listing *) 0)->native), "platform_dir_listing has room for a WIN32_FIND_DATAA");

static void listing_take_name(platform_dir_listing *listing)
{
    listing->name = ((const WIN32_FIND_DATAA *) listing->native)->cFileName;
}

bool platform_dir_first(platform_dir_listing *listing, const char *pattern)
{
    listing->handle = FindFirstFileA(pattern, (WIN32_FIND_DATAA *) listing->native);
    if (listing->handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    listing_take_name(listing);
    return true;
}

bool platform_dir_next(platform_dir_listing *listing)
{
    if (!FindNextFileA(listing->handle, (WIN32_FIND_DATAA *) listing->native)) {
        return false;
    }
    listing_take_name(listing);
    return true;
}

void platform_dir_close(platform_dir_listing *listing)
{
    FindClose(listing->handle);
}

int32_t platform_desktop_width(void)
{
    return GetSystemMetrics(SM_CXSCREEN);
}

int32_t platform_desktop_height(void)
{
    return GetSystemMetrics(SM_CYSCREEN);
}

int32_t platform_desktop_color_depth(void)
{
    HDC dc = GetDC(0);
    GetDeviceCaps(dc, NUMCOLORS);
    int32_t planes = GetDeviceCaps(dc, PLANES);
    int32_t depth = planes * GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(0, dc);
    return depth;
}

void platform_desktop_palette(void *entries, int32_t count)
{
    GetPaletteEntries(0, 0, (UINT) count, (PALETTEENTRY *) entries);
}

int32_t platform_memory_available(void)
{
    MEMORYSTATUS status;
    memset(&status, 0, sizeof(status));
    status.dwLength = sizeof(status);
    GlobalMemoryStatus(&status);
    return (int32_t) status.dwAvailPhys;
}

bool platform_time_counter(uint64_t *count)
{
    LARGE_INTEGER value;
    if (!QueryPerformanceCounter(&value)) {
        return false;
    }
    *count = (uint64_t) value.QuadPart;
    return true;
}

bool platform_time_frequency(uint64_t *frequency)
{
    LARGE_INTEGER value;
    value.QuadPart = 0;
    bool available = QueryPerformanceFrequency(&value) != 0;
    *frequency = (uint64_t) value.QuadPart;
    return available;
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
