// The building sounds of the city view (city_view.c): start, views of a drawn building, halving and the play tick,
// with their rules and the sample of each channel slot.

#ifndef SOUND_CITY_VIEW_H
#define SOUND_CITY_VIEW_H

#include "sound/city_sound_types.h"

#include <stdint.h>

enum city_sound_rule {
    CITY_SOUND_RULE_VIEWS_THRESHOLD = 0,
    CITY_SOUND_RULE_DELAY_MS = 1,
    CITY_SOUND_RULE_UPDATE_MS = 2,
    CITY_SOUND_RULE_PAN_VIEWS = 3,
    CITY_SOUND_RULE_COUNT = 4,
};

enum city_sound_slots {
    CITY_SOUND_SLOTS = 64,
};

extern int32_t city_sound_rules[CITY_SOUND_RULE_COUNT];
extern int32_t city_sound_channel_ids[CITY_SOUND_SLOTS];

void sound_city_init(void);
void sound_city_decay_views(void);
void sound_city_mark_building_view(int32_t building_id, int32_t direction);
void sound_city_play(void);

#endif
