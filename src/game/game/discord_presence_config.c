// The config table of the Discord presence: discord.json (the Discord application's id and whether the presence is
// on), described over its C record.

#include "core/config.h"
#include "game/discord_presence.h"

#define DISCORD_PRESENCE_STRIDE ((int32_t) (sizeof(discord_presence_settings) / sizeof(int32_t)))

static const config_name discord_presence_names[] = {
    { "discord", DISCORD_PRESENCE_DISCORD },
    { 0, 0 },
};

static const config_field discord_presence_fields[] = {
    { "app_id", (int32_t *) discord_presence_config[0].app_id, DISCORD_PRESENCE_STRIDE, DISCORD_RPC_APP_ID_SIZE, 0, 0, 0, 0, CONFIG_WIDTH_TEXT },
    { "enabled", &discord_presence_config[0].enabled, DISCORD_PRESENCE_STRIDE, 1, 0, 1, 0 },
};

static const config_storage discord_presence_storage[] = {
    { discord_presence_config, sizeof(discord_presence_config) },
};

const config_table discord_presence_table = {
    "discord.json",
    "presence",
    discord_presence_names,
    DISCORD_PRESENCE_RECORDS,
    CONFIG_TABLE_COLUMNS(discord_presence_fields, discord_presence_storage),
};
