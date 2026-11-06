// The intermezzo (a picture between screens with the mission's speech, until its time is up or a right click), the
// campaign's mission choice (the current link's choices as markers on its map), the briefing's scroll buttons, the
// end-of-game screens and the small helpers of starting a mission.
//
// A mission choice whose link is not in the campaign is not accepted.

#include "window/intermezzo.h"
#include "city/city_player.h"
#include "figure/action.h"
#include "game/game_data.h"
#include "graphics/arrow_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "input/input_data.h"
#include "platform/platform_time.h"
#include "scenario/campaign.h"
#include "scenario/campaign_load.h"
#include "sound/music.h"
#include "sound/sound_play.h"
#include "text/lang.h"
#include "video/video_data.h"
#include "widget/widget_data.h"
#include "window/hotkeys.h"
#include "window/message_dialog.h"
#include "window/mission_flow.h"
#include "window/player_selection.h"
#include "window/victory_dialog.h"
#include "window/window_id.h"
#include "window/window_image_group.h"
#include <stdint.h>

enum intermezzo_layout {
    INTERMEZZO_FULL_SCREEN_WIDTH = 1024,
    INTERMEZZO_FULL_SCREEN_HEIGHT = 768,
    MISSION_NONE = 0x7F,
    MISSION_FIRST_BRIEFING = 200,
    MISSION_CLASSICAL_BRIEFING_SPLIT = 439,
    MISSION_CLASSICAL_BRIEFING_HIGH_OFFSET = 15,
    MISSION_DEFAULT_TRANSITION_TEXT = 37,
    MISSION_SPEECH_FILE_LENGTH = 40,
    MISSION_CHOICE_MARKER_SIZE = 44,
    MISSION_CHOICE_TEXT_GROUP = 144,
    MISSION_CHOICE_TEXT_NONE = 0,
    MARKER_NORMAL = 0,
    MARKER_HOVERED = 1,
    MARKER_CHOSEN = 2,
    BRIEFING_TOOLTIP_SCROLL_UP = 12,
    BRIEFING_TOOLTIP_SCROLL_DOWN = 13,
    SOUND_STREAM_MUSIC_FADE = 1,
};

static campaign_link *current_link(void)
{
    int32_t link = settings_data.campaign_link;
    if (link < 0 || link >= campaign_object.campaign.link_count) {
        return 0;
    }
    return &campaign_object.campaign.links[link];
}

static void set_full_screen_offsets(void)
{
    image_view_x = (INTERMEZZO_FULL_SCREEN_WIDTH - g_screen_width) / 2;
    image_view_width = g_screen_width;
    image_view_y = (INTERMEZZO_FULL_SCREEN_HEIGHT - g_screen_height) / 2;
    image_view_height = g_screen_height;
}

static void end_picture_draw(int32_t image)
{
    image_draw(image, ui_origin_x, ui_origin_y);
}

void window_end_picture_1_draw(void)
{
    end_picture_draw(image_group(IMAGE_GROUP_END_PICTURE_1));
}

void window_end_picture_2_draw(void)
{
    end_picture_draw(image_group(IMAGE_GROUP_END_PICTURE_2));
}

void window_end_picture_3_draw(void)
{
    end_picture_draw(image_group(IMAGE_GROUP_END_PICTURE_3));
}

void window_end_picture_final_draw(void)
{
    victory_x = ui_origin_x + 96;
    victory_y = ui_origin_y + 160;
    set_full_screen_offsets();
    image_draw(image_group(IMAGE_GROUP_END_PICTURE_FINAL), 0, 0);
    outer_panel_draw(victory_x, victory_y, 28, 10);
    lang_text_draw_centered(LANG_GROUP_MISSION_GOALS, VICTORY_TEXT_CAMPAIGN_COMPLETE, victory_x + 32, victory_y + 24, 384, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    lang_text_draw_multiline(LANG_GROUP_MISSION_GOALS, VICTORY_TEXT_CAMPAIGN_COMPLETE_MESSAGE, victory_x + 32, victory_y + 80, 384, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

int32_t mission_briefing_message(void)
{
    if (settings_data.is_custom_scenario == 0) {
        const campaign_link *link = current_link();
        if (link != 0) {
            int32_t intro = link->intro_id;
            if (settings_data.classical_names == 0) {
                return intro;
            }
            if (intro >= MISSION_CLASSICAL_BRIEFING_SPLIT) {
                return intro + MISSION_CLASSICAL_BRIEFING_HIGH_OFFSET;
            }
            return intro + MISSION_FIRST_BRIEFING;
        }
    }
    if (settings_data.mission_id == MISSION_NONE) {
        return MISSION_FIRST_BRIEFING;
    }
    return settings_data.mission_id + MISSION_FIRST_BRIEFING;
}

int32_t mission_transition_text(void)
{
    if (settings_data.is_custom_scenario == 0) {
        const campaign_link *link = current_link();
        if (link != 0) {
            return link->transition_text_id;
        }
    }
    return MISSION_DEFAULT_TRANSITION_TEXT;
}

void mission_start_intermezzo(void)
{
    window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, WINDOW_BRIEFING, INTERMEZZO_DURATION_MS);
    mission_briefing_started = 0;
    rich_scroll_pos = 0;
    rich_line_count = 0;
    dialog_text_id = mission_briefing_message();
}

static void mission_choice_accept(void)
{
    if (campaign_mission_choice.choice == 0) {
        return;
    }
    int8_t next = (int8_t) campaign_find_link(&campaign_object.campaign, (int8_t) settings_data.campaign, settings_data.campaign_link, campaign_mission_choice.choice - 1);
    if (next < 0 || next >= campaign_object.campaign.link_count) {
        return;
    }
    mission_records_store();
    settings_data.previous_campaign_link = (uint8_t) settings_data.campaign_link;
    settings_data.campaign_link = next;
    settings_data.mission_id = (int8_t) campaign_object.campaign.links[next].mission_id;
    mission_start_intermezzo();
}

static void mission_start_city(void)
{
    city_player *player = city_player_current();
    if (window_current_id == WINDOW_BRIEFING) {
        player->mission_saved_game_state = 0;
    }
    window_current_id = WINDOW_CITY;
    sound_stream_fade_out(SOUND_STREAM_MUSIC_FADE);
    int32_t *difficulty = &player->difficulty;
    sound_music_playing_special = 0;
    if (settings_data.difficulty < *difficulty) {
        *difficulty = settings_data.difficulty;
    }
}

void mission_button_accept(void)
{
    focus_button_id_secondary = 0;
    focus_button_id = 0;
    image_button_focus_id = 0;
    focus_button_id_tertiary = 0;
    city_overlay_previous = 0;
    if (window_current_id == WINDOW_NAME_INPUT) {
        player_selection_create_player();
    } else if (window_current_id == WINDOW_MISSION_CHOICE) {
        mission_choice_accept();
    } else {
        mission_start_city();
    }
    window_redraw_request = 1;
}

void mission_button_back(void)
{
    if (window_current_id == WINDOW_BRIEFING && (int8_t) settings_data.previous_campaign_link >= 0) {
        window_current_id = WINDOW_MISSION_CHOICE;
        settings_data.campaign_link = (int8_t) settings_data.previous_campaign_link;
        settings_data.previous_campaign_link = (uint8_t) -1;
    }
    window_redraw_request = 1;
}

void mission_show_briefing(void)
{
    if (settings_data.is_custom_scenario != 0) {
        return;
    }
    window_intermezzo_show(INTERMEZZO_MISSION_BRIEFING, WINDOW_BRIEFING_2, INTERMEZZO_DURATION_MS);
    rich_scroll_pos = 0;
    rich_line_count = 0;
    tooltip_text_id = 0;
    dialog_scroll_drag_y = 0;
    window_redraw_request = 1;
}

void mission_briefing_draw_scroll(void)
{
    window_ui_redraw = 1;
    if (dialog_scroll_max == 0) {
        return;
    }
    image_buttons_draw(ui_panel_x + 554, ui_panel_y + 168, message_scroll_up.button.buttons, 1);
    image_buttons_draw(ui_panel_x + 554, ui_panel_y + 382, message_scroll_down.button.buttons, 1);
    message_dialog_draw_scrollbar_dot();
}

static int32_t scroll_button_handled(int32_t y, void *buttons, int32_t tooltip)
{
    image_buttons_set_tooltip(ui_panel_x + 560, y, buttons, 1, tooltip);
    return image_buttons_handle_mouse(ui_panel_x + 560, y, buttons, 1) != 0;
}

void mission_briefing_handle_scroll(void)
{
    if (scroll_button_handled(ui_panel_y + 168, message_scroll_up.button.buttons, BRIEFING_TOOLTIP_SCROLL_UP)) {
        return;
    }
    if (scroll_button_handled(ui_panel_y + 382, message_scroll_down.button.buttons, BRIEFING_TOOLTIP_SCROLL_DOWN)) {
        return;
    }
    message_dialog_handle_scrollbar();
}

static int32_t mouse_on_choice(const campaign_choice *choice)
{
    return mouse_inside_rect(choice->x + ui_origin_x, choice->y + ui_origin_y, MISSION_CHOICE_MARKER_SIZE, MISSION_CHOICE_MARKER_SIZE) != 0;
}

void mission_choice_click(void)
{
    if (mouse_left_went_up == 0) {
        return;
    }
    const campaign_link *link = current_link();
    for (int32_t i = 0; i < link->choice_count; i++) {
        if (mouse_on_choice(&link->choices[i])) {
            campaign_mission_choice.choice = (uint8_t) (i + 1);
            window_redraw_request = 1;
            return;
        }
    }
}

void mission_choice_draw_markers(void)
{
    const campaign_link *link = current_link();
    if (link == 0) {
        return;
    }
    for (int32_t i = 0; i < link->choice_count; i++) {
        const campaign_choice *choice = &link->choices[i];
        int32_t state;
        if (mouse_on_choice(choice)) {
            state = MARKER_HOVERED;
        } else {
            state = campaign_mission_choice.choice - 1 != i ? MARKER_NORMAL : MARKER_CHOSEN;
        }
        image_draw(image_group(IMAGE_GROUP_MISSION_CHOICE_MARKERS) + state, choice->x + ui_origin_x, choice->y + ui_origin_y);
    }
}

void window_mission_choice_draw(void)
{
    const campaign_link *link = current_link();
    if (link == 0) {
        return;
    }
    ui_panel_x = ui_origin_x;
    ui_panel_y = ui_origin_y;
    set_full_screen_offsets();
    image_draw(image_group(IMAGE_GROUP_MISSION_CHOICE_BACKGROUND), 0, 0);
    image_draw(link->background_id + image_group(IMAGE_GROUP_MISSION_CHOICE_LINKS), ui_panel_x, ui_panel_y);
    lang_text_draw(MISSION_CHOICE_TEXT_GROUP, link->text_id, ui_panel_x + 20, ui_panel_y + 410, image_group(IMAGE_GROUP_FONT_LARGE), 0);
    int32_t text = campaign_mission_choice.choice == 0 ? MISSION_CHOICE_TEXT_NONE : link->choices[campaign_mission_choice.choice - 1].text_id;
    lang_text_draw_multiline(MISSION_CHOICE_TEXT_GROUP, text, ui_panel_x + 20, ui_panel_y + 440, 560, image_group(IMAGE_GROUP_FONT_BLACK), 0);
}

static void play_mission_speech(const char *files)
{
    sound_speech_play_mission(&files[settings_data.mission_id * MISSION_SPEECH_FILE_LENGTH]);
}

static void draw_mission_briefing(void)
{
    if (settings_data.is_custom_scenario != 0) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND) + 1, 0, 0);
    } else if (settings_data.mission_id != MISSION_NONE) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND) + 1, 0, 0);
        sound_music_fading = 1;
        sound_music_playing_special = 1;
        play_mission_speech(settings_data.classical_names != 0 ? mission_briefing_speech_files_classic : mission_briefing_speech_files);
    }
    window_redraw_request = 1;
}

static void draw_mission_won(void)
{
    sound_music_fading = 1;
    sound_music_playing_special = 1;
    if (settings_data.is_custom_scenario != 0) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND), 0, 0);
    } else if (settings_data.mission_id != MISSION_NONE) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND), 0, 0);
        play_mission_speech(settings_data.classical_names != 0 ? mission_victory_speech_files_classic : mission_victory_speech_files);
    }
    window_redraw_request = 1;
}

int32_t window_intermezzo_show(int32_t type, int32_t next_window, int32_t duration_ms)
{
    uint32_t now = platform_time_ms();
    intermezzo_type = type;
    intermezzo_end_ms = (int32_t) (now + (uint32_t) duration_ms);
    intermezzo_next_window = next_window;
    window_current_id = WINDOW_INTERMEZZO;
    window_redraw_request = 1;
    return intermezzo_end_ms;
}

void window_intermezzo_draw(void)
{
    set_full_screen_offsets();
    if (intermezzo_type == INTERMEZZO_MISSION_BRIEFING) {
        draw_mission_briefing();
    } else if (intermezzo_type == INTERMEZZO_FIRED) {
        image_draw(image_group(IMAGE_GROUP_INTERMEZZO_BACKGROUND), 0, 0);
    } else if (intermezzo_type == INTERMEZZO_MISSION_WON) {
        draw_mission_won();
    }
}

void window_intermezzo_handle(void)
{
    uint32_t now = platform_time_ms();
    if ((int32_t) now > intermezzo_end_ms || mouse_right_went_up != 0) {
        window_redraw_request = 1;
        window_current_id = (uint8_t) intermezzo_next_window;
    }
}
