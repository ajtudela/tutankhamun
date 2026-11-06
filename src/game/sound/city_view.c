// The building sounds of the city view: drawing a building in use counts views for its channel, an orientation change
// and the passing time halve the counts, and the play tick takes the channel waiting longest once it has the views
// and its delay passed, walks its samples and restarts its counts. No building sample is loaded, so the tick plays
// nothing. Entertainment venues are silent while enemies are in the city or when unstaffed. The rules come from
// config/city_sound_rules.json and config/city_sound_channels.json.

#include "sound/city_view.h"

#include "building/building.h"
#include "building/building_type.h"
#include "city/city_player.h"
#include "core/calc.h"
#include "io/io_data.h"
#include "map/grid.h"
#include "platform/platform_time.h"
#include "sound/sound_data.h"
#include "widget/widget_data.h"

#include <string.h>


uint32_t city_sound_last_update;

enum city_sound_direction {
    CITY_SOUND_DIRECTION_LEFT = 0,
    CITY_SOUND_DIRECTION_MIDDLE = 2,
    CITY_SOUND_DIRECTION_RIGHT = 4,
};

int32_t city_sound_rules[CITY_SOUND_RULE_COUNT] = {
    [CITY_SOUND_RULE_VIEWS_THRESHOLD] = 200,
    [CITY_SOUND_RULE_DELAY_MS] = 30000,
    [CITY_SOUND_RULE_UPDATE_MS] = 2000,
    [CITY_SOUND_RULE_PAN_VIEWS] = 10,
};

int32_t city_sound_channel_ids[CITY_SOUND_SLOTS] = {
    0, 30, 34, 38, 42, 46, 50, 51, 52, 53, 54, 55, 56, 57, 58, 62, 63, 64, 65, 66, 67, 68, 69, 70, 74, 78, 79, 80, 81, 82, 83, 87,
    89, 91, 93, 95, 97, 98, 99, 100, 101, 102, 106, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 132, 133,
};

static void city_sound_setup_slot(int32_t index, int32_t channel_id)
{
    city_sound_channel *ch = &city_sounds.channels[index];
    ch->views_threshold = city_sound_rules[CITY_SOUND_RULE_VIEWS_THRESHOLD];
    ch->num_channels = 1;
    ch->channels[0] = channel_id;
    ch->in_use = 1;
    ch->delay_millis = (uint32_t) city_sound_rules[CITY_SOUND_RULE_DELAY_MS];
}

void sound_city_init(void)
{
    uint32_t now = platform_time_ms();
    city_sound_last_update = now;
    memset(&city_sounds, 0, sizeof(city_sounds));
    for (int32_t i = 1; i < CITY_SOUND_CHANNELS; i++) {
        city_sounds.channels[i].last_played_time = now;
    }
    for (int32_t i = 1; i < CITY_SOUND_SLOTS; i++) {
        city_sound_setup_slot(i, city_sound_channel_ids[i]);
    }
}

static int32_t is_silent_venue(int32_t type)
{
    return (type == BUILDING_BANDSTAND) || (type == BUILDING_BOOTH) || (type == BUILDING_SENET_HOUSE) || (type == BUILDING_PAVILION) || (type == BUILDING_ZOO);
}

void sound_city_mark_building_view(int32_t building_id, int32_t direction)
{
    building *b = building_get(building_id);
    if (b->state != BUILDING_STATE_IN_USE) {
        return;
    }
    int16_t type = b->type;
    int32_t channel = building_type_to_city_channel[type];
    if (channel < 1) {
        return;
    }
    if (is_silent_venue(type)) {
        if (city_player_current()->enemies > 0) {
            return;
        }
        if (b->num_workers < 1) {
            return;
        }
    }
    city_sound_channel *ch = &city_sounds.channels[channel];
    ch->available = 1;
    ch->total_views += 1;
    ch->direction_views[direction] += 1;
}

static void clear_direction_views(city_sound_channel *ch)
{
    for (int32_t d = 0; d < CITY_SOUND_DIRECTIONS; d++) {
        ch->direction_views[d] = 0;
    }
}

void sound_city_decay_views(void)
{
    for (int32_t i = 1; i < CITY_SOUND_CHANNELS; i++) {
        city_sound_channel *ch = &city_sounds.channels[i];
        clear_direction_views(ch);
        ch->total_views /= 2;
    }
}

static void mark_playable_channels(uint32_t now)
{
    for (int32_t i = 1; i < CITY_SOUND_CHANNELS; i++) {
        city_sound_channel *ch = &city_sounds.channels[i];
        ch->should_play = 0;
        if (ch->available < 1) {
            ch->total_views = 0;
            clear_direction_views(ch);
        } else {
            ch->available = 0;
            if ((ch->total_views >= ch->views_threshold) && ((int32_t) (now - ch->last_played_time) >= (int32_t) ch->delay_millis)) {
                ch->should_play = 1;
            }
        }
    }
}

static int32_t longest_waiting_channel(uint32_t now)
{
    int32_t max_age = 0;
    int32_t max_id = 0;
    for (int32_t i = 1; i < CITY_SOUND_CHANNELS; i++) {
        const city_sound_channel *ch = &city_sounds.channels[i];
        if (ch->should_play > 0) {
            int32_t age = (int32_t) (now - ch->last_played_time);
            if (age > max_age) {
                max_age = age;
                max_id = i;
            }
        }
    }
    return max_id;
}

static void cycle_sample_channels(city_sound_channel *pick)
{
    for (int32_t tries = 0; tries < CITY_SOUND_SAMPLES; tries++) {
        pick->current_channel += 1;
        if (pick->current_channel >= pick->num_channels) {
            pick->current_channel = 0;
        }
    }
}

void sound_city_play(void)
{
    uint32_t now = (uint32_t) time_current_millis;
    mark_playable_channels(now);
    if ((int32_t) (now - city_sound_last_update) < city_sound_rules[CITY_SOUND_RULE_UPDATE_MS]) {
        return;
    }
    int32_t max_id = longest_waiting_channel(now);
    if (max_id < 1) {
        return;
    }
    city_sound_channel *pick = &city_sounds.channels[max_id];
    cycle_sample_channels(pick);
    now = (uint32_t) time_current_millis;
    city_sound_last_update = now;
    pick->last_played_time = now;
    pick->total_views = 0;
    clear_direction_views(pick);
    pick->times_played += 1;
}
