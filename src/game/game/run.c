// The game frame in two parts: the tick part (frame counter, time, animation flags, the loop of elapsed ticks with
// the mission autosave and the fast-forward to the end of the year) and the frame part (windows, drawing, input,
// cursor, music and ambient sounds), with the city window's one random number per frame and the cursor that follows
// the selected legion or ship.
//
// Without a selected ship the arrow is set, and the action cursor of figure 0 then replaces it.

#include "game/run.h"

#include "building/building_type.h"
#include "city/city_player.h"
#include "city/message.h"
#include "core/crt_compat.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "figure/formation_orders.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "game/tick.h"
#include "game/time.h"
#include "game/transport_cursor.h"
#include "graphics/graphics_present.h"
#include "graphics/ui_layout.h"
#include "input/mouse.h"
#include "io/game_file.h"
#include "platform/platform_message.h"
#include "platform/platform_time.h"
#include "sound/ambient.h"
#include "sound/city_play.h"
#include "sound/city_view.h"
#include "sound/music.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_play.h"
#include "widget/city_view.h"
#include "widget/sidebar.h"
#include "widget/tooltip.h"
#include "widget/top_menu.h"
#include "widget/widget_data.h"
#include "window/window_draw.h"
#include "window/window_id.h"
#include "window/window_input.h"

#include <stdbool.h>
#include <stdint.h>

int32_t game_frame_counter;

enum run_rule {
    RUN_CITY_SOUND_RANDOM_RANGE = 50,
    RUN_CRT_RAND_LIMIT = 0x8000,
    RUN_TOOLTIP_KIND_CITY_SELECT = 0x23,
    RUN_INVASION_MUSIC_ENEMIES = 16,
    RUN_INVASION_MUSIC_WARSHIPS = 5,
};

static void cancel_fast_forward_on_rbutton_up(void)
{
    platform_message msg;
    if (!platform_message_peek(&msg, PLATFORM_MSG_RIGHT_UP, PLATFORM_MSG_RIGHT_UP, false)) {
        return;
    }
    game_fast_forward_year = 0;
}

void game_run_tick(void)
{
    game_frame_counter++;
    time_current_millis = (int32_t) platform_time_ms();
    game_animation_update();
    int32_t n = game_get_elapsed_ticks();
    for (int32_t i = 0; i < n; i++) {
        game_tick_counter++;
        game_tick_run();
        if (game_fast_forward_year != 0) {
            cancel_fast_forward_on_rbutton_up();
        }
        bool last_tick = game_fast_forward_year == 0;
        game_file_write_mission_saved_game();
        if (last_tick) {
            break;
        }
    }
}

void game_run_consume_city_rand(void)
{
    if (window_current_id != WINDOW_CITY) {
        return;
    }
    int32_t r = crt_rand();
    if ((r * RUN_CITY_SOUND_RANDOM_RANGE) / RUN_CRT_RAND_LIMIT == 1) {
        sound_city_play_random();
    }
}

void game_run_draw_window(void)
{
    window_draw_background_dispatch(0);
    ui_layout_refresh();
    window_draw_foreground_dispatch();
}

static int32_t legion_layout_cursor(int32_t layout)
{
    switch (layout) {
        case FORMATION_DOUBLE_LINE_1:
            return CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_1;
        case FORMATION_DOUBLE_LINE_2:
            return CURSOR_SHAPE_LAYOUT_DOUBLE_LINE_2;
        case FORMATION_SINGLE_LINE_1:
            return CURSOR_SHAPE_LAYOUT_SINGLE_LINE_1;
        case FORMATION_SINGLE_LINE_2:
            return CURSOR_SHAPE_LAYOUT_SINGLE_LINE_2;
        case FORMATION_CHARGE:
            return CURSOR_SHAPE_LAYOUT_CHARGE;
        default:
            return CURSOR_SHAPE_LEGION_MOVE;
    }
}

static int32_t legion_cursor(void)
{
    const formation *legion = formation_get(formation_selected);
    if (figure_fighter_at_grid_offset(map_info.selected_grid) == 0) {
        return legion_layout_cursor(legion->layout);
    }
    return legion->figure_type == FIGURE_INFANTRY ? CURSOR_SHAPE_INFANTRY_ATTACK : CURSOR_SHAPE_ATTACK;
}

static int32_t ship_land_cursor(int32_t figure_id)
{
    return transport_cursor_can_land(figure_id) ? CURSOR_SHAPE_SHIP_LAND : CURSOR_SHAPE_SHIP_BLOCKED;
}

static int32_t ship_embark_cursor(int32_t figure_id)
{
    return transport_cursor_can_embark(figure_id) ? CURSOR_SHAPE_SHIP_EMBARK : CURSOR_SHAPE_SHIP_BLOCKED;
}

static void set_transport_cursor(void)
{
    int32_t figure_id = selected_transport_id;
    if (selected_transport_id < 1) {
        cursor_set(CURSOR_SHAPE_ARROW);
    }
    int32_t shape;
    switch (city_figures.records[figure_id].action_state) {
        case FIGURE_ACTION_TRANSPORT_SHIP_SAILING:
            shape = transport_cursor_can_embark(figure_id) ? CURSOR_SHAPE_SHIP_EMBARK : ship_land_cursor(figure_id);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_EVADING:
            shape = CURSOR_SHAPE_TRANSPORT_EVADING;
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_DOCK:
            if (selected_transport_active != 0) {
                shape = CURSOR_SHAPE_SHIP_BLOCKED;
            } else if (transport_cursor_can_embark(figure_id)) {
                shape = CURSOR_SHAPE_SHIP_EMBARK;
            } else if (transport_cursor_can_land(figure_id)) {
                shape = CURSOR_SHAPE_SHIP_LAND;
            } else {
                return;
            }
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LEGION:
            shape = ship_embark_cursor(figure_id);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_GOING_TO_LANDING:
            shape = ship_land_cursor(figure_id);
            break;
        case FIGURE_ACTION_TRANSPORT_SHIP_DOCKED:
            if (selected_transport_active == 0) {
                return;
            }
            shape = CURSOR_SHAPE_TRANSPORT_DOCKED;
            break;
        default:
            shape = CURSOR_SHAPE_SHIP_MOVE;
            break;
    }
    cursor_set(shape);
}

static void set_warship_cursor(void)
{
    if (figure_fighter_at_grid_offset(map_info.selected_grid) != 0) {
        cursor_set(CURSOR_SHAPE_ATTACK);
        return;
    }
    if (selected_warship_id < 1) {
        cursor_set(CURSOR_SHAPE_ARROW);
    }
    int32_t shape;
    switch (city_figures.records[selected_warship_id].action_state) {
        case FIGURE_ACTION_WARSHIP_HUNTING:
            shape = CURSOR_SHAPE_WARSHIP_HUNTING;
            break;
        case FIGURE_ACTION_WARSHIP_DOCKED:
            if (selected_warship_active == 0) {
                return;
            }
            shape = CURSOR_SHAPE_WARSHIP_DOCKED;
            break;
        case FIGURE_ACTION_WARSHIP_PATROLLING:
            shape = CURSOR_SHAPE_SHIP_MOVE;
            break;
        default:
            shape = CURSOR_SHAPE_ARROW;
            break;
    }
    cursor_set(shape);
}

static void set_cursor_shape(void)
{
    switch (window_current_id) {
        case WINDOW_CITY_LEGION_SELECTED:
            cursor_set(legion_cursor());
            break;
        case WINDOW_CITY_LEGION_ORDER:
            cursor_set(CURSOR_SHAPE_LEGION_ORDER);
            break;
        case WINDOW_CITY:
            cursor_set(construction_building_type == BUILDING_CLEAR_LAND ? CURSOR_SHAPE_CLEAR_LAND : CURSOR_SHAPE_ARROW);
            break;
        case WINDOW_CITY_TRANSPORT_SELECTED:
            set_transport_cursor();
            break;
        case WINDOW_CITY_WARSHIP_SELECTED:
            set_warship_cursor();
            break;
        default:
            cursor_set(CURSOR_SHAPE_ARROW);
            break;
    }
}

void game_run_update_music_and_ambient(void)
{
    const city_player *player = city_player_current();
    if (player->enemies < RUN_INVASION_MUSIC_ENEMIES && player->enemy_warships < RUN_INVASION_MUSIC_WARSHIPS) {
        if (music_object.invasion_music_request == 1) {
            sound_music_fading = 1;
            music_object.invasion_music_request = 0;
        }
        if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_MUSIC) == 0 && sound_music_playing_special == 0) {
            if (window_current_id == WINDOW_CITY) {
                sound_music_update(&music_object);
            }
        }
    } else if (sound_digital_ok != 0 && music_object.invasion_music_request == 0 && settings_data.sound_music_enabled != 0) {
        sound_lib_play_stream(&sound_manager, battle_music_path, SOUND_STREAM_MUSIC, 0);
        music_object.invasion_music_request = 1;
    }

    game_run_consume_city_rand();

    if (city_locusts_countdown == 0 && city_hailstorm_countdown == 0 && city_frogs_countdown == 0) {
        if (scroll_in_progress == 0) {
            if (window_current_id == WINDOW_CITY || window_current_id == WINDOW_BUILDING_INFO || window_current_id == WINDOW_MESSAGE_DIALOG) {
                sound_ambient_update();
            } else if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_CITY) != 0) {
                sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
            }
        } else {
            sound_stream_fade_down(SOUND_STREAM_CITY);
        }
    }
    if (sound_music_fading != 0) {
        sound_stream_fade_down(SOUND_STREAM_MUSIC);
    }
    if (sound_speech_fading != 0) {
        sound_stream_fade_down(SOUND_STREAM_SPEECH);
    }
}

void game_run_frame(void)
{
    mouse_update();
    window_draw_background_dispatch(0);
    ui_layout_refresh();
    window_draw_foreground_dispatch();
    window_handle_input_dispatch();
    mouse_clear_clicks();
    tooltip_handle();
    tooltip_handle_overlay();
    widget_top_menu_draw(0);
    widget_sidebar_draw_number_of_messages(1);
    city_warning_draw();
    if (tooltip_text_id != 0) {
        tooltip_draw();
        if (tooltip_type == RUN_TOOLTIP_KIND_CITY_SELECT) {
            city_view_mouse_select();
        }
        tooltip_reset();
    }
    debug_draw_network_info();
    graphics_refresh_screen(0);
    sound_city_play();
    set_cursor_shape();
    game_run_update_music_and_ambient();
}

void game_run(void)
{
    game_run_tick();
    game_run_frame();
}
