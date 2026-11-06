// The tables of sound/sound_data.c.

#ifndef SOUND_SOUND_DATA_H
#define SOUND_SOUND_DATA_H

#include <stdint.h>

#include "graphics/button.h"
#include "sound/ambient_types.h"
#include "sound/city_channel_types.h"
#include "sound/music_types.h"
#include "sound/sound_lib_types.h"

enum sound_data_size {
    CITY_CHANNEL_TABLE_SIZE = 300,
    MAP_SELECTION_IMAGE_BUTTONS = 4,
    MAP_SELECTION_DIFFICULTY_ARROWS = 2,
    MAP_SELECTION_LIST_BUTTONS = 15,
};

extern int32_t building_type_to_city_channel[CITY_CHANNEL_TABLE_SIZE];

typedef struct {
    image_button custom_image_buttons[MAP_SELECTION_IMAGE_BUTTONS];
    image_button campaign_image_buttons[MAP_SELECTION_IMAGE_BUTTONS];
    generic_button kind_toggle;
    int32_t unread_after_toggle;
    image_button difficulty_option_arrows[MAP_SELECTION_DIFFICULTY_ARROWS];
    generic_button custom_list[MAP_SELECTION_LIST_BUTTONS];
    int32_t unread_after_custom_list;
    generic_button campaign_list[MAP_SELECTION_LIST_BUTTONS];
    int32_t unread_after_campaign_list;
} map_selection_button_layout;

extern map_selection_button_layout map_selection_button_data;

extern int32_t ambient_monument_threshold;

extern char phrase_names_walkers[24480];

extern const char phrase_names_zookeeper[20][24];
extern float sound_lib_defaults[78];

extern const char str_wave_out_failed[164];

extern music_state music_object;

extern sound_lib_state sound_manager;

extern int32_t figure_channels[166];

extern int32_t effect_channels[102];

extern ambient_channel_table ambient_channel_base;

extern city_channel_table city_channel_base;

extern char speech_walker_path[304];

extern int32_t ambient_current_type;

extern int32_t sound_digital_ok;

extern sound_lib_state *sound_lib_manager;

#endif
