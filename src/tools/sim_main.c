// tutankhamun_sim: the game without a window, video or sound, for the saved game regression
// (docs/architecture-plan.md, 1.2):
//
//     tutankhamun_sim <game dir> in.sav --days N --out out.sav [--seed S] [--log file]
//
// It starts the game as the program does (app_startup: configs, texts, models, tables), loads the save, runs the
// simulation tick (game_tick_run) 51 times a day for N game days (16 to a month, 192 to a year) and saves through the
// game's own save. The frame part
// of the game is not run: no windows, drawing or input, no city window random number per frame, no music, and no
// mission autosave into the game folder. The ticks do not wait for the clock, and the clock the game reads is the
// run's own (bound with platform_bind: a fixed start, 40 ms a tick; the predators reseed rand() from it), so a run
// gives the same save every time. A message box is logged and answered OK. Sound and video go to libraries that do
// nothing (the wave output does not open, so the game turns digital sound off; no video opens). --seed sets the C run
// time's random seed (1 by default), which the saves do not keep.
//
//     tutankhamun_sim --diff-saves a.sav b.sav
//
// compares two saves of this build's layout chunk by chunk (docs/architecture-plan.md, 1.3): for each chunk that
// differs, how many of its elements differ and where the first difference is.
//
// Exit codes: 0 done (with --diff-saves: the saves are the same); 1 a usage error, or the saves differ; 2 the game does
// not start, or a save does not load, read or write.

#include "app/image.h"
#include "app/media_lib.h"
#include "core/core_data.h"
#include "game/game_data.h"
#include "game/tick.h"
#include "graphics/graphics_data.h"
#include "io/chunk_io.h"
#include "io/game_file.h"
#include "io/io_data.h"
#include "platform/log.h"
#include "platform/platform_file.h"
#include "platform/platform_win32.h"
#include "widget/widget_data.h"
#include "platform/version.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum sim_limits {
    SIM_TICKS_PER_DAY = 51,
    SIM_PATH_SIZE = 1024,
    SIM_EXIT_USAGE = 1,
    SIM_EXIT_FAILED = 2,
    SIM_CLOCK_START_MS = 1000000,
    SIM_CLOCK_MS_PER_TICK = 40,
    SIM_MESSAGE_BOX_OK = 1,
};

// --- the media libraries that do nothing ---

static void MEDIA_API none_i(int32_t a)
{
    (void) a;
}
static void MEDIA_API none_ii(int32_t a, int32_t b)
{
    (void) a, (void) b;
}
static void MEDIA_API none_iiii(int32_t a, int32_t b, int32_t c, int32_t d)
{
    (void) a, (void) b, (void) c, (void) d;
}
static void MEDIA_API none_iiiiiii(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e, int32_t f, int32_t g)
{
    (void) a, (void) b, (void) c, (void) d, (void) e, (void) f, (void) g;
}
static void MEDIA_API none_ifff(int32_t a, float b, float c, float d)
{
    (void) a, (void) b, (void) c, (void) d;
}
static void MEDIA_API none_icp(int32_t a, const char *b, void *c)
{
    (void) a, (void) b, (void) c;
}
static void MEDIA_API none_p(void *a)
{
    (void) a;
}
static void MEDIA_API none_v(void)
{
}
static int32_t MEDIA_API zero_v(void)
{
    return 0;
}
static int32_t MEDIA_API zero_i(int32_t a)
{
    (void) a;
    return 0;
}
static int32_t MEDIA_API zero_p(void *a)
{
    (void) a;
    return 0;
}
static int32_t MEDIA_API zero_ip(int32_t a, void *b)
{
    (void) a, (void) b;
    return 0;
}
static int32_t MEDIA_API zero_ipi(int32_t a, void *b, int32_t c)
{
    (void) a, (void) b, (void) c;
    return 0;
}
static int32_t MEDIA_API zero_icp(int32_t a, const char *b, const void *c)
{
    (void) a, (void) b, (void) c;
    return 0;
}
static int32_t MEDIA_API zero_pci(void *a, const char *b, int32_t c)
{
    (void) a, (void) b, (void) c;
    return 0;
}
static int32_t MEDIA_API zero_ppp(int32_t *a, int32_t *b, char **c)
{
    (void) a, (void) b, (void) c;
    return 0;
}
static int32_t MEDIA_API zero_ui(uint32_t a, int32_t b)
{
    (void) a, (void) b;
    return 0;
}
static int32_t MEDIA_API sample_done(int32_t a)
{
    (void) a;
    return SMP_DONE;
}
// the wave output does not open: the game turns digital sound off
static int32_t MEDIA_API wave_out_fails(void **driver, void **info, int32_t device, void *format)
{
    (void) info, (void) device, (void) format;
    *driver = 0;
    return 1;
}
static void *MEDIA_API null_cp(const char *a, void *b)
{
    (void) a, (void) b;
    return 0;
}

static const media_sound_lib silent_sound = {
    .ail_set_3d_position = none_iiii,
    .ail_start_3d_sample = none_i,
    .ail_set_3d_provider_preference = zero_icp,
    .ail_set_3d_sample_file = zero_ip,
    .ail_set_3d_sample_volume = none_ii,
    .ail_3d_sample_status = sample_done,
    .ail_enumerate_3d_providers = zero_ppp,
    .ail_open_3d_provider = zero_i,
    .ail_3d_provider_attribute = none_icp,
    .ail_stream_volume = zero_i,
    .ail_open_stream = zero_pci,
    .ail_3d_update_position = none_ii,
    .ail_stream_status = sample_done,
    .ail_set_stream_volume = none_ii,
    .ail_set_sample_loop_count = none_ii,
    .ail_set_sample_pan = none_ii,
    .ail_set_3d_orientation = none_iiiiiii,
    .ail_set_3d_sample_loop_count = none_ii,
    .ail_start_sample = none_i,
    .ail_file_read = null_cp,
    .ail_init_sample = none_i,
    .ail_set_sample_file = zero_ipi,
    .ail_sample_status = sample_done,
    .ail_sample_loop_count = zero_i,
    .ail_end_3d_sample = none_i,
    .ail_end_sample = none_i,
    .ail_close_stream = none_i,
    .ail_close_3d_provider = none_i,
    .ail_shutdown = none_v,
    .ail_startup = zero_v,
    .ail_set_preference = zero_ui,
    .ail_wave_out_open = wave_out_fails,
    .ail_release_3d_sample_handle = none_i,
    .ail_allocate_3d_sample_handle = zero_i,
    .ail_release_sample_handle = none_i,
    .ail_allocate_sample_handle = zero_p,
    .ail_set_sample_volume = none_ii,
    .ail_set_sample_reverb = none_ifff,
    .ail_set_stream_loop_count = none_ii,
    .ail_start_stream = none_i,
    .ail_mem_free_lock = none_p,
};

static int32_t MEDIA_API video_zero_pu(void *a, uint32_t b)
{
    (void) a, (void) b;
    return 0;
}
static void *MEDIA_API video_null_u(uint32_t a)
{
    (void) a;
    return 0;
}
static const char *MEDIA_API video_error(void)
{
    return "no video in tutankhamun_sim";
}
static void *MEDIA_API video_null_cu(const char *a, uint32_t b)
{
    (void) a, (void) b;
    return 0;
}
static uint32_t MEDIA_API video_zero_u_p(void *a)
{
    (void) a;
    return 0;
}
static int32_t MEDIA_API video_copy(void *a, void *b, int32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g)
{
    (void) a, (void) b, (void) c, (void) d, (void) e, (void) f, (void) g;
    return 0;
}
static void MEDIA_API video_to_buffer(void *a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, void *f, uint32_t g)
{
    (void) a, (void) b, (void) c, (void) d, (void) e, (void) f, (void) g;
}
static int32_t MEDIA_API video_zero_u(uint32_t a)
{
    (void) a;
    return 0;
}
static void *MEDIA_API video_null_cuu(const char *a, uint32_t b, uint32_t c)
{
    (void) a, (void) b, (void) c;
    return 0;
}

static const media_video_lib no_video = {
    .bink_do_frame = zero_p,
    .bink_set_sound_system = video_zero_pu,
    .bink_open_miles = video_null_u,
    .bink_wait = zero_p,
    .bink_get_error = video_error,
    .bink_next_frame = none_p,
    .bink_open = video_null_cu,
    .bink_service = zero_p,
    .bink_dd_surface_type = video_zero_u_p,
    .bink_copy_to_buffer = video_copy,
    .bink_close = none_p,
    .bink_buffer_close = none_p,
    .smack_to_buffer = video_to_buffer,
    .smack_wait = zero_p,
    .smack_sound_use_mss = video_zero_u,
    .smack_open = video_null_cuu,
    .smack_close = none_p,
    .smack_do_frame = zero_p,
    .smack_next_frame = none_p,
};

// --- the clock and the message box of the run ---

// the game reads the clock in its ticks too (the predators reseed rand() from it, as the original does): the run's
// clock starts at a fixed time and moves a fixed step per tick
static uint32_t sim_clock_ms = SIM_CLOCK_START_MS;

static uint32_t sim_clock(void)
{
    return sim_clock_ms;
}

static int32_t sim_message_box(void *window, const char *text, const char *caption, uint32_t type)
{
    (void) window, (void) type;
    platform_log("sim: message box %s: %s", caption ? caption : "", text ? text : "");
    return SIM_MESSAGE_BOX_OK;
}

static const platform_bindings sim_bindings = {
    .time_ms = sim_clock,
    .tick_count = sim_clock,
    .message_box = sim_message_box,
};

// --- the comparison of two saves ---

typedef struct {
    int32_t fd;
    uint8_t *data;
} save_reader;

// the chunk as this build writes it: compressed, or its bytes as they are
static int32_t read_chunk(save_reader *r, const chunk_entry *c, int32_t size)
{
    if (c->compressed) {
        return io_read_compressed_chunk(r->fd, r->data, size);
    }
    return platform_file_read(r->fd, r->data, (uint32_t) size) == size;
}

static int32_t diff_saves(const char *a_path, const char *b_path)
{
    save_reader a = { platform_file_open(a_path, PLATFORM_OPEN_READ), malloc(CHUNK_MAX_SIZE) };
    save_reader b = { platform_file_open(b_path, PLATFORM_OPEN_READ), malloc(CHUNK_MAX_SIZE) };
    int32_t result = SIM_EXIT_FAILED;
    if (a.fd == -1 || b.fd == -1 || !a.data || !b.data) {
        fprintf(stderr, "cannot open %s\n", a.fd == -1 ? a_path : b_path);
        goto done;
    }
    int32_t differing = 0;
    for (int32_t i = 0; i < SAVEGAME_CHUNKS; i++) {
        const chunk_entry *c = &savegame_chunk_table[i];
        int32_t size = c->element_size * c->count;
        if (size > CHUNK_MAX_SIZE || !read_chunk(&a, c, size) || !read_chunk(&b, c, size)) {
            fprintf(stderr, "chunk %d (%d x %d bytes) does not read: the saves are not of this build's layout\n", i, c->count, c->element_size);
            goto done;
        }
        int32_t first = -1, elements = 0, last_element = -1;
        for (int32_t at = 0; at < size; at++) {
            if (a.data[at] == b.data[at]) {
                continue;
            }
            if (first < 0) {
                first = at;
            }
            int32_t element = at / c->element_size;
            if (element != last_element) {
                elements++;
                last_element = element;
            }
        }
        if (first >= 0) {
            differing++;
            printf("chunk %3d: %d of %d elements of %d bytes differ, the first at offset %d (element %d, byte %d)\n", i, elements, c->count, c->element_size,
                first, first / c->element_size, first % c->element_size);
        }
    }
    printf(differing ? "%d chunks differ\n" : "the saves are the same\n", differing);
    result = differing ? SIM_EXIT_USAGE : 0;
done:
    if (a.fd != -1) {
        platform_file_close(a.fd);
    }
    if (b.fd != -1) {
        platform_file_close(b.fd);
    }
    free(a.data);
    free(b.data);
    return result;
}

// --- the run ---

static void print_usage(void)
{
    fprintf(stderr,
        "usage: tutankhamun_sim <game dir> in.sav --days N --out out.sav [--seed S] [--log file]\n"
        "       tutankhamun_sim --diff-saves a.sav b.sav\n");
}

static int32_t parse_count(const char *text, int32_t *value)
{
    char *end;
    long number = strtol(text, &end, 10);
    if (end == text || *end != 0 || number < 0 || number > 1000000) {
        return 0;
    }
    *value = (int32_t) number;
    return 1;
}

int main(int argc, char **argv)
{
    // before anything of the game: the static initializers register the sound's shutdown for the exit
    media_sound = silent_sound;
    media_video = no_video;
    platform_bind(&sim_bindings);
    if (argc > 1 && strcmp(argv[1], "--diff-saves") == 0) {
        if (argc != 4) {
            print_usage();
            return SIM_EXIT_USAGE;
        }
        if (!image_init()) {
            return SIM_EXIT_FAILED;
        }
        // the static initializers fill the chunk table (core/static_init.c); a compressed chunk is read through the
        // spare screen, which the game's start-up allocates
        image_run_static_initializers();
        spare_screen = malloc(CHUNK_MAX_SIZE);
        return diff_saves(argv[2], argv[3]);
    }
    const char *game_dir = 0, *in = 0, *out = 0, *log = "tutankhamun_sim.log";
    int32_t days = -1, seed = 1;
    for (int32_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--days") == 0 && i + 1 < argc) {
            if (!parse_count(argv[++i], &days)) {
                print_usage();
                return SIM_EXIT_USAGE;
            }
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            if (!parse_count(argv[++i], &seed)) {
                print_usage();
                return SIM_EXIT_USAGE;
            }
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            out = argv[++i];
        } else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc) {
            log = argv[++i];
        } else if (!game_dir) {
            game_dir = argv[i];
        } else if (!in) {
            in = argv[i];
        } else {
            print_usage();
            return SIM_EXIT_USAGE;
        }
    }
    if (!game_dir || !in || !out || days < 0) {
        print_usage();
        return SIM_EXIT_USAGE;
    }
    // the paths of the command line are taken before the game folder becomes the current one
    static char in_path[SIM_PATH_SIZE], out_path[SIM_PATH_SIZE], log_path[SIM_PATH_SIZE];
    platform_path_absolute(in, in_path, sizeof(in_path));
    platform_path_absolute(out, out_path, sizeof(out_path));
    platform_path_absolute(log, log_path, sizeof(log_path));
    platform_log_open(log_path);
    platform_log("sim: %s", version_text());
    if (!image_init() || !platform_dir_enter(game_dir) || !app_startup()) {
        fprintf(stderr, "the game does not start in %s, see %s\n", game_dir, log_path);
        return SIM_EXIT_FAILED;
    }
    if (game_file_load_saved_game(in_path) != 0) {
        fprintf(stderr, "cannot load %s, see %s\n", in_path, log_path);
        return SIM_EXIT_FAILED;
    }
    crt_rand_seed = (uint32_t) seed;
    platform_log("sim: %s loaded, day %d.%d.%d, %d days to run", in_path, game_time_day, game_time_month, game_time_year, days);
    for (int64_t tick = 0; tick < (int64_t) days * SIM_TICKS_PER_DAY; tick++) {
        sim_clock_ms += SIM_CLOCK_MS_PER_TICK;
        time_current_millis = (int32_t) sim_clock_ms;
        game_tick_counter++;
        game_tick_run();
    }
    platform_log("sim: day %d.%d.%d", game_time_day, game_time_month, game_time_year);
    if (game_file_io_write_saved_game(out_path) != 0) {
        fprintf(stderr, "cannot write %s, see %s\n", out_path, log_path);
        return SIM_EXIT_FAILED;
    }
    printf("%s: %d days, now day %d month %d year %d\n", out_path, days, game_time_day, game_time_month, game_time_year);
    return 0;
}
