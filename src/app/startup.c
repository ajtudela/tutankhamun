// The game's start-up without a display, sound or window: the gameplay configs of the config folder beside the
// program (without it the start-up fails with "no configs"), the C runtime's static initializers, then the settings,
// texts, models and tables a saved game is loaded into. A play may ask for another display mode than the settings'.

#include "app/image.h"
#include "building/building_data.h"
#include "building/model_load.h"
#include "building/monument_part_offsets.h"
#include "core/config.h"
#include "core/random.h"
#include "empire/trade_recommends.h"
#include "game/startup_load.h"
#include "graphics/graphics_data.h"
#include "input/input_data.h"
#include "platform/log.h"
#include "platform/platform_data.h"
#include "scenario/campaign_load.h"
#include "scenario/reason_phrases.h"
#include "sound/sound_init.h"
#include "text/lang_load.h"
#include "widget/widget_data.h"
#include "window/display_resolution_list.h"

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void platform_init_without_display(void)
{
    int32_t mode = settings_data.display_mode;
    int32_t width = mode == 2 ? 800 : mode == 3 ? 1024 : 640;
    int32_t height = mode == 2 ? 600 : mode == 3 ? 768 : 480;
    display_resolution_custom_screen(&width, &height);
    random_generator_state.iv1 = 0x54657687;
    random_generator_state.iv2 = 0x72641663;
    random_generator_state.iv3 = 0x34518632;
    g_shadow_c = 0;
    text_draw_shadow = 0;
    mouse_drag_state = 0;
    g_screen_width = width;
    ui_origin_x = (width - 640) / 2;
    g_screen_height = height;
    ui_origin_y = (height - 480) / 2;
    g_screen_pitch = width * 2;
    display_width = width;
    screen_height = height;
    g_clip_x0 = 0;
    g_clip_x1 = width;
    g_clip_y0 = 0;
    g_clip_y1 = height;
    desktop_width = GetSystemMetrics(SM_CXSCREEN);
    desktop_height = GetSystemMetrics(SM_CYSCREEN);
    screen_rgb565_ptr = calloc(1, (size_t) (width * 2 * height));
    platform_log("startup: screen %dx%d without a display", width, height);
}

static int32_t step(const char *name, int32_t ok)
{
    if (!ok) {
        platform_log("startup: %s failed", name);
    }
    return ok;
}

static int32_t no_configs_message_box;
static int32_t requested_display_mode;

void app_request_display_mode(int32_t mode)
{
    requested_display_mode = mode;
}

static void apply_requested_display_mode(void)
{
    if (requested_display_mode != 0) {
        settings_data.display_mode = requested_display_mode;
        settings_data.display_mode_start = requested_display_mode;
    }
}

void app_report_no_configs_in_box(void)
{
    no_configs_message_box = 1;
}

static int32_t report_no_configs(const char *dir)
{
    char text[MAX_PATH + 160];
    wsprintfA(text, "no configs: the folder %s is missing.\nThe config folder must be beside tutankhamun.exe.", dir);
    platform_log("startup: %s", text);
    fprintf(stderr, "%s\n", text);
    if (no_configs_message_box) {
        MessageBoxA(0, text, "tutankhamun: no configs", MB_OK | MB_ICONERROR);
    }
    return 0;
}

static int32_t load_configs_beside_program(void)
{
    char dir[MAX_PATH];
    DWORD length = GetModuleFileNameA(0, dir, MAX_PATH);
    char *slash = length > 0 && length < MAX_PATH ? strrchr(dir, '\\') : 0;
    if (!slash || (size_t) (slash - dir) + sizeof("\\config") > MAX_PATH) {
        return report_no_configs("<the program's folder is not known>\\config");
    }
    lstrcpyA(slash, "\\config");
    DWORD attributes = GetFileAttributesA(dir);
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return report_no_configs(dir);
    }
    platform_log("startup: configs from %s", dir);
    config_load_all(dir);
    return 1;
}

int32_t app_startup(void)
{
    if (!load_configs_beside_program()) {
        return 0;
    }
    image_run_static_initializers();
    if (!step("settings", settings_load()) || !step("texts", lang_load(text_file_name, message_file_name))) {
        return 0;
    }
    settings_data.cd_drive_letter = DEFAULT_CD_DRIVE_LETTER;
    settings_load_default_names();
    apply_requested_display_mode();
    platform_init_without_display();
    int32_t setup = game_setup();
    if (!step("setup", setup != 1 && setup != 2 && setup != 3 && setup != -2) || !step("building models", building_model_load()) ||
        !step("figure models", figure_model_load()) || !step("tax sentiment", tax_sentiment_model_load()) ||
        !step("campaign", campaign_load(&campaign_object.campaign, campaign_file_name)) || !step("trade recommendations", trade_recommends_load()) ||
        !step("event phrases", scenario_events_load_reason_phrases())) {
        return 0;
    }
    sound_init();
    editor_active = 0;
    editor_map_preview = 0;
    editor_brush_size = 2;
    editor_tool_kind = 0;
    overlay_show_all = 0;
    construction_placing_royal_tomb = 0;
    cheat_allow_all = 0;
    debug_event_text = 0;
    debug_event_level = 1;
    debug_event_kind = 0;
    name_list_choice = -1;
    game_state_init();
    abu_simbel_rotate_part_descs(&abu_simbel_object);
    library_rotate_part_descs(&library_object);
    caesareum_rotate_part_descs(&caesareum_object);
    lighthouse_rotate_parts(&lighthouse_object);
    small_royal_tomb_rotate_part_descs(&small_royal_tomb_object.base);
    medium_royal_tomb_rotate_parts(&medium_royal_tomb_object.base);
    large_royal_tomb_rotate_parts(&large_royal_tomb_object.base);
    grand_royal_tomb_rotate_parts(&grand_royal_tomb_object.base);
    sun_temple_init_part_offsets();
    mausoleum_init_part_offsets();
    record_savings.finished.savings = 0;
    sidebar_init_positions();
    window_current_id = 0x44;
    window_redraw_request = 1;
    platform_measure_speed();
    platform_log("startup: done");
    return 1;
}
