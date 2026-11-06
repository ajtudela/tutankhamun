// The gameplay config tables of the sound (core/config.h): the city sound rules (views, delay, play interval,
// panning) and the sample each building sound channel plays.

#include "core/config.h"
#include "sound/city_view.h"

static const config_name city_sound_rule_names[] = {
    { "views_threshold", CITY_SOUND_RULE_VIEWS_THRESHOLD },
    { "delay_ms", CITY_SOUND_RULE_DELAY_MS },
    { "update_ms", CITY_SOUND_RULE_UPDATE_MS },
    { "pan_views", CITY_SOUND_RULE_PAN_VIEWS },
    { 0, 0 },
};

static const config_field city_sound_rule_fields[] = {
    { "value", city_sound_rules, 1, 1, 0, 1000000, 0 },
};

static const config_storage city_sound_rule_storage[] = {
    { city_sound_rules, sizeof(city_sound_rules) },
};

const config_table city_sound_rules_table = {
    "city_sound_rules.json",
    "rule",
    city_sound_rule_names,
    CITY_SOUND_RULE_COUNT,
    CONFIG_TABLE_COLUMNS(city_sound_rule_fields, city_sound_rule_storage),
};

static const config_field city_sound_channel_fields[] = {
    { "channel", city_sound_channel_ids, 1, 1, 0, 1000, 0 },
};

static const config_storage city_sound_channel_storage[] = {
    { city_sound_channel_ids, sizeof(city_sound_channel_ids) },
};

const config_table city_sound_channels_table = {
    "city_sound_channels.json",
    "slot",
    0,
    CITY_SOUND_SLOTS,
    CONFIG_TABLE_COLUMNS(city_sound_channel_fields, city_sound_channel_storage),
};
