// The Bink functions for the play mode (media_lib.h's video library) on top of Akhenaten's Bink 1 decoder
// (src/third_party/akhenaten_bink): BinkOpen maps the game's Binks\ paths to the install's Video\ files, the handle
// exposes the BINK struct's first fields, frames are decoded in order and converted to the game's 16-bit screen format,
// and the audio is played through SDL_mixer. While a video is open the display surfaces are a small stand-in over the
// game's frame buffer. The game's own player (src/game/video/bink.c) stays as it is.
//
// TUTANKHAMUN_VIDEO=none: no video is played (BinkOpen fails, the game skips the video).
//
// The game relies on two details of the Bink library:
// - frame_num is 1-based, the frame BinkDoFrame decodes next, as in the BINK struct the game reads;
// - when the low-resolution files are not installed, the high-resolution file of the same video is played.

#include "app/media_lib.h"
#include "app/play.h"
#include "building/building_data.h"
#include "graphics/graphics_data.h"
#include "platform/platform_file.h"
#include "video/video_data.h"
#include "window/window_data.h"
#include "platform/platform_screen.h"
#include "platform/platform_display.h"
#include "platform/screen.h"
#include "third_party/akhenaten_bink/bink_c.h"
#include "video/bink.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"


#define BINKSURFACE555 9
#define BINKSURFACE565 10

#define HANDLE_MAGIC 0x4B4E4942u

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t stretch_width;
    uint32_t stretch_height;
    uint32_t frames;
    uint32_t frame_num;
    uint32_t magic;
    akh_bink *movie;
    int32_t fd;
    char path[PLATFORM_PATH_MAX];
    int32_t decoded;
    int32_t ok;
    uint32_t micros_per_frame;
    uint64_t start;
    int32_t started;
    int32_t frames_decoded;
    int32_t frames_shown;
    int32_t frames_late;
    int32_t audio_bytes;
    uint32_t open_ms;
} video;

static struct {
    int32_t checked;
    int32_t none;
    uint64_t frequency;
    int32_t surface_in;
    platform_surface *saved_primary;
    platform_surface *saved_back;
    int32_t saved_fullscreen;
    int32_t last_type;
    SDL_mutex *mutex;
    SDL_AudioStream *stream;
    int32_t hooked;
    int32_t mixer_channels;
} vb;

static void settings(void)
{
    if (vb.checked) {
        return;
    }
    vb.checked = 1;
    const char *v = getenv("TUTANKHAMUN_VIDEO");
    vb.none = v && platform_text_compare_nocase(v, "none") == 0;
    vb.frequency = SDL_GetPerformanceFrequency();
    akh_bink_set_log(play_log);
}

static double ms_since(const uint64_t *t)
{
    uint64_t now = SDL_GetPerformanceCounter();
    return (double) (now - *t) * 1000.0 / (double) vb.frequency;
}

static video *handle_of(void *p)
{
    video *v = p;
    return v && v->magic == HANDLE_MAGIC ? v : 0;
}

static int32_t surface_lock(platform_surface *surface, platform_frame *frame)
{
    (void) surface;
    if (!screen_rgb565_ptr) {
        return PLATFORM_SURFACE_FAILED;
    }
    frame->pixels = (uint8_t *) screen_rgb565_ptr;
    frame->pitch = g_screen_width * 2;
    frame->width = g_screen_width;
    frame->height = g_screen_height;
    return PLATFORM_SURFACE_OK;
}

static int32_t surface_unlock(platform_surface *surface)
{
    (void) surface;
    platform_screen_update();
    platform_screen_render();
    return PLATFORM_SURFACE_OK;
}

static int32_t surface_fill(platform_surface *surface, uint16_t colour)
{
    (void) surface;
    if (screen_rgb565_ptr) {
        uint16_t *p = screen_rgb565_ptr;
        for (int32_t i = 0; i < g_screen_width * g_screen_height; i++) {
            p[i] = colour;
        }
    }
    return PLATFORM_SURFACE_OK;
}

static int32_t surface_present(platform_surface *surface, platform_surface *source)
{
    (void) surface;
    (void) source;
    return PLATFORM_SURFACE_OK;
}

static bool surface_is_lost(platform_surface *surface)
{
    (void) surface;
    return false;
}

static int32_t surface_restore(platform_surface *surface)
{
    (void) surface;
    return PLATFORM_SURFACE_OK;
}

static const platform_surface_ops surface_ops = { surface_lock, surface_unlock, surface_fill, surface_present, surface_is_lost, surface_restore };
static platform_surface surface_object = { &surface_ops };

static void surface_install(void)
{
    if (vb.surface_in || display_front_surface || display_back_surface) {
        return;
    }
    vb.saved_primary = display_front_surface;
    vb.saved_back = display_back_surface;
    vb.saved_fullscreen = settings_data.fullscreen;
    display_front_surface = &surface_object;
    display_back_surface = &surface_object;
    settings_data.fullscreen = play_fullscreen_for_video();
    vb.surface_in = 1;
}

static void surface_remove(void)
{
    if (!vb.surface_in) {
        return;
    }
    display_front_surface = vb.saved_primary;
    display_back_surface = vb.saved_back;
    settings_data.fullscreen = vb.saved_fullscreen;
    play_fullscreen_after_video();
    vb.surface_in = 0;
}

static void SDLCALL audio_hook(void *userdata, Uint8 *stream, int len)
{
    (void) userdata;
    int32_t got = 0;
    SDL_LockMutex(vb.mutex);
    if (vb.stream) {
        got = SDL_AudioStreamGet(vb.stream, stream, len);
    }
    SDL_UnlockMutex(vb.mutex);
    if (got < 0) {
        got = 0;
    }
    if (got < len) {
        memset(stream + got, 0, (size_t) (len - got));
    }
}

static void audio_open(video *v)
{
    int32_t channels = akh_bink_audio_channels(v->movie);
    int32_t rate = akh_bink_audio_rate(v->movie);
    int32_t frequency = 0, mixer_channels = 0;
    Uint16 format = 0;
    if (channels <= 0 || rate <= 0) {
        play_log("video: %s has no audio", v->path);
        return;
    }
    if (!Mix_QuerySpec(&frequency, &format, &mixer_channels)) {
        play_log("video: no audio device (SDL_mixer not open)");
        return;
    }
    if (!vb.mutex) {
        vb.mutex = SDL_CreateMutex();
    }
    SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S16SYS, (Uint8) channels, rate, format, (Uint8) mixer_channels, frequency);
    if (!stream) {
        play_log("video: no audio stream (%s)", SDL_GetError());
        return;
    }
    SDL_LockMutex(vb.mutex);
    vb.stream = stream;
    SDL_UnlockMutex(vb.mutex);
    Mix_HookMusic(audio_hook, 0);
    vb.hooked = 1;
    play_log("video: audio %d Hz, %d channel(s) -> mixer %d Hz, %d channel(s)", rate, channels, frequency, mixer_channels);
}

static void audio_close(void)
{
    if (vb.hooked) {
        Mix_HookMusic(0, 0);
        vb.hooked = 0;
    }
    if (vb.mutex) {
        SDL_LockMutex(vb.mutex);
    }
    if (vb.stream) {
        SDL_FreeAudioStream(vb.stream);
        vb.stream = 0;
    }
    if (vb.mutex) {
        SDL_UnlockMutex(vb.mutex);
    }
}

static void audio_put(video *v)
{
    int32_t bytes = 0;
    const void *pcm = akh_bink_frame_audio(v->movie, &bytes);
    if (!pcm || bytes <= 0) {
        return;
    }
    v->audio_bytes += bytes;
    if (!vb.stream) {
        return;
    }
    SDL_LockMutex(vb.mutex);
    SDL_AudioStreamPut(vb.stream, pcm, bytes);
    SDL_UnlockMutex(vb.mutex);
}

#if defined(_WIN32)
static HHOOK message_hook;

static LRESULT CALLBACK message_hook_proc(int code, WPARAM wparam, LPARAM lparam)
{
    const MSG *m = (const MSG *) lparam;
    if (code == HC_ACTION && wparam == PM_REMOVE && m && (m->message == WM_KEYDOWN || m->message == WM_SYSKEYDOWN || m->message == WM_LBUTTONDOWN)) {
        play_log("video: message 0x%04X (wParam 0x%X) during the video: the game's loop ends it", m->message, (uint32_t) m->wParam);
    }
    return CallNextHookEx(message_hook, code, wparam, lparam);
}

static void message_hook_install(void)
{
    if (!message_hook) {
        message_hook = SetWindowsHookExA(WH_GETMESSAGE, message_hook_proc, 0, GetCurrentThreadId());
    }
}

static void message_hook_remove(void)
{
    if (message_hook) {
        UnhookWindowsHookEx(message_hook);
        message_hook = 0;
    }
}
#else
static void message_hook_install(void)
{
}

static void message_hook_remove(void)
{
}
#endif

static int32_t file_exists(const char *path)
{
    return platform_file_exists(path) && !platform_dir_exists(path);
}

static const char *find_nocase(const char *text, const char *part)
{
    size_t n = strlen(part);
    for (const char *p = text; *p; p++) {
        if (platform_text_compare_nocase_n(p, part, n) == 0) {
            return p;
        }
    }
    return 0;
}

static int32_t map_path(const char *path, char *out, size_t size)
{
    if (file_exists(path)) {
        snprintf(out, size, "%s", path);
        return 1;
    }
    const char *binks = find_nocase(path, "binks\\");
    if (!binks) {
        return 0;
    }
    const char *rest = binks + 6;
    snprintf(out, size, "Video\\%s", rest);
    if (file_exists(out)) {
        return 1;
    }
    const char *name = strrchr(rest, '\\');
    name = name ? name + 1 : rest;
    char base[PLATFORM_PATH_MAX];
    snprintf(base, sizeof(base), "%s", name);
    char *dot = strrchr(base, '.');
    if (dot) {
        *dot = 0;
    }
    size_t n = strlen(base);
    if (n > 6 && platform_text_compare_nocase(base + n - 6, "_small") == 0) {
        base[n - 6] = 0;
    }
    if (n < 4 || platform_text_compare_nocase(base + strlen(base) - 4, "_big") != 0) {
        snprintf(out, size, "Video\\High\\%s_big.bik", base);
    } else {
        snprintf(out, size, "Video\\High\\%s.bik", base);
    }
    return file_exists(out);
}

static video *open_videos[4];

static void *MEDIA_API bink_open(const char *path, uint32_t flags)
{
    (void) flags;
    settings();
    if (vb.none) {
        play_log("video: no video (BinkOpen %s)", path ? path : "(null)");
        return 0;
    }
    char file[PLATFORM_PATH_MAX];
    if (!path || !map_path(path, file, sizeof(file))) {
        play_log("video: no file for BinkOpen %s", path ? path : "(null)");
        return 0;
    }
    char native[PLATFORM_PATH_MAX];
#if defined(__ANDROID__)
    int32_t fd = platform_file_open(file, PLATFORM_OPEN_READ);
    snprintf(native, sizeof(native), "fd:%d", (int) fd);
    akh_bink *movie = fd >= 0 ? akh_bink_open(native) : 0;
    if (!movie && fd >= 0) {
        platform_file_close(fd);
    }
#else
    int32_t fd = -1;
    akh_bink *movie = akh_bink_open(platform_path_native(file, native, sizeof(native)));
#endif
    if (!movie) {
        play_log("video: the decoder cannot open %s (BinkOpen %s)", file, path);
        return 0;
    }
    video *v = calloc(1, sizeof(video));
    v->magic = HANDLE_MAGIC;
    v->movie = movie;
    v->fd = fd;
    snprintf(v->path, sizeof(v->path), "%s", file);
    v->width = v->stretch_width = (uint32_t) akh_bink_width(movie);
    v->height = v->stretch_height = (uint32_t) akh_bink_height(movie);
    v->frames = (uint32_t) akh_bink_frames(movie);
    v->frame_num = 1;
    v->ok = 1;
    v->micros_per_frame = (uint32_t) akh_bink_micros_per_frame(movie);
    v->open_ms = SDL_GetTicks();
    for (int32_t i = 0; i < 4; i++) {
        if (!open_videos[i]) {
            open_videos[i] = v;
            break;
        }
    }
    play_log("video: BinkOpen %s -> %s: %ux%u, %d frames (%u shown at most), %u us per frame", path, file, v->width, v->height, akh_bink_frames(movie),
        v->frames, v->micros_per_frame);
    surface_install();
    audio_open(v);
    message_hook_install();
    return v;
}

static void MEDIA_API bink_close(void *handle)
{
    video *v = handle_of(handle);
    int32_t known = 0;
    for (int32_t i = 0; i < 4; i++) {
        if (v && open_videos[i] == v) {
            open_videos[i] = 0;
            known = 1;
        }
    }
    if (!known) {
        return;
    }
    double seconds = v->started ? ms_since(&v->start) / 1000.0 : 0.0;
    play_log("video: closed %s: %d frames decoded, %d shown, %d late, of %u; %.1f s playing (%.1f s at the movie's "
                 "rate), %d audio bytes",
        v->path, v->frames_decoded, v->frames_shown, v->frames_late, v->frames, seconds, (double) v->frames_decoded * v->micros_per_frame / 1e6,
        v->audio_bytes);
    audio_close();
    surface_remove();
    message_hook_remove();
    akh_bink_close(v->movie);
    if (v->fd >= 0) {
        platform_file_close(v->fd);
    }
    v->magic = 0;
    free(v);
}

static int32_t MEDIA_API bink_do_frame(void *handle)
{
    video *v = handle_of(handle);
    if (!v || !v->ok) {
        return 0;
    }
    int32_t want = (int32_t) v->frame_num;
    if (want == v->decoded) {
        return 0;
    }
    int32_t ok;
    if (want == 1 || want < v->decoded) {
        ok = akh_bink_first_frame(v->movie);
        v->decoded = 1;
    } else {
        ok = 1;
        while (ok && v->decoded < want) {
            ok = akh_bink_next_frame(v->movie);
            v->decoded++;
        }
    }
    if (!ok) {
        play_log("video: %s: frame %d not decoded", v->path, want);
        v->ok = 0;
        return 0;
    }
    if (!v->started) {
        v->start = SDL_GetPerformanceCounter();
        v->started = 1;
    } else if (ms_since(&v->start) > (double) (want - 1) * v->micros_per_frame / 1000.0 + 2.0 * v->micros_per_frame / 1000.0) {
        v->frames_late++;
    }
    v->frames_decoded++;
    audio_put(v);
    return 0;
}

static void MEDIA_API bink_next_frame(void *handle)
{
    video *v = handle_of(handle);
    if (v) {
        v->frame_num = v->frame_num >= v->frames ? 1 : v->frame_num + 1;
    }
}

static int32_t MEDIA_API bink_wait(void *handle)
{
    video *v = handle_of(handle);
    if (!v || !v->started) {
        return 0;
    }
    double due = (double) ((int32_t) v->frame_num - 1) * v->micros_per_frame / 1000.0;
    if (ms_since(&v->start) < due) {
        SDL_Delay(1);
        return 1;
    }
    return 0;
}

static int32_t MEDIA_API bink_service(void *handle)
{
    (void) handle;
    return 0;
}

static const char *MEDIA_API bink_get_error(void)
{
    return "";
}

static int32_t MEDIA_API bink_set_sound_system(void *open, uint32_t param)
{
    (void) open;
    (void) param;
    return 1;
}

static void *MEDIA_API bink_open_miles(uint32_t param)
{
    (void) param;
    return 0;
}

static uint32_t MEDIA_API bink_dd_surface_type(void *surface)
{
    (void) surface;
    vb.last_type = platform_screen_is_565() ? BINKSURFACE565 : BINKSURFACE555;
    return (uint32_t) vb.last_type;
}

static void MEDIA_API bink_buffer_close(void *buffer)
{
    (void) buffer;
}

static int32_t MEDIA_API bink_copy_to_buffer(void *handle, void *dest, int32_t pitch, uint32_t height, uint32_t x, uint32_t y, uint32_t flags)
{
    video *v = handle_of(handle);
    const uint32_t *rgba = v ? akh_bink_frame_rgba(v->movie) : 0;
    if (!rgba || !dest) {
        return 1;
    }
    int32_t type = (int32_t) (flags & 0xF);
    int32_t w = (int32_t) v->width;
    int32_t h = (int32_t) (height < v->height ? height : v->height);
    const uint8_t *buffer = (const uint8_t *) screen_rgb565_ptr;
    size_t buffer_size = (size_t) g_screen_width * (size_t) g_screen_height * 2;
    uint8_t *base = (uint8_t *) dest + (size_t) y * (size_t) pitch + (size_t) x * 2;
    for (int32_t row = 0; row < h; row++) {
        uint16_t *out = (uint16_t *) (base + (size_t) row * (size_t) pitch);
        if (buffer && ((const uint8_t *) out < buffer || (const uint8_t *) (out + w) > buffer + buffer_size)) {
            continue;
        }
        const uint32_t *in = rgba + (size_t) row * (size_t) w;
        if (type == BINKSURFACE565) {
            for (int32_t i = 0; i < w; i++) {
                uint32_t p = in[i];
                out[i] = (uint16_t) (((p >> 8) & 0xF800) | ((p >> 5) & 0x07E0) | ((p >> 3) & 0x001F));
            }
        } else {
            for (int32_t i = 0; i < w; i++) {
                uint32_t p = in[i];
                out[i] = (uint16_t) (((p >> 9) & 0x7C00) | ((p >> 6) & 0x03E0) | ((p >> 3) & 0x001F));
            }
        }
    }
    v->frames_shown++;
    return 0;
}

static void *MEDIA_API smack_open(const char *path, uint32_t flags, uint32_t extra)
{
    (void) flags;
    (void) extra;
    play_log("video: no video (SmackOpen %s)", path ? path : "(null)");
    return 0;
}

static void MEDIA_API smack_close(void *clip)
{
}

static int32_t MEDIA_API smack_wait(void *clip)
{
    return 0;
}

static int32_t MEDIA_API smack_do_frame(void *clip)
{
    return 0;
}

static void MEDIA_API smack_next_frame(void *clip)
{
}

static int32_t MEDIA_API smack_sound_use_mss(uint32_t driver)
{
    return 0;
}

static void MEDIA_API smack_to_buffer(void *clip, uint32_t x, uint32_t y, uint32_t pitch, uint32_t height, void *buffer, uint32_t flags)
{
}

const media_video_lib play_video_library = {
    .bink_do_frame = bink_do_frame,
    .bink_set_sound_system = bink_set_sound_system,
    .bink_open_miles = bink_open_miles,
    .bink_wait = bink_wait,
    .bink_get_error = bink_get_error,
    .bink_next_frame = bink_next_frame,
    .bink_open = bink_open,
    .bink_service = bink_service,
    .bink_dd_surface_type = bink_dd_surface_type,
    .bink_copy_to_buffer = bink_copy_to_buffer,
    .bink_close = bink_close,
    .bink_buffer_close = bink_buffer_close,
    .smack_to_buffer = smack_to_buffer,
    .smack_wait = smack_wait,
    .smack_sound_use_mss = smack_sound_use_mss,
    .smack_open = smack_open,
    .smack_close = smack_close,
    .smack_do_frame = smack_do_frame,
    .smack_next_frame = smack_next_frame,
};

void video_play_intro(void)
{
    video_player *player = video_player_ptr;
    if (player) {
        int32_t y = (g_screen_height - 0x11E) / 2;
        int32_t x = (g_screen_width - 0x1E0) / 2;
        video_play(player, settings_data.cpu < 0xE9 ? "Binks\\low\\intro.bik" : "Binks\\high\\intro_big.bik", x, y);
    }
}
