// Discord Rich Presence of a normal play: the mission's title as its briefing shows it (a custom map: the map's name,
// else the player's) over the date as "Year N, <Egyptian month>", sent after a game or a map is loaded and at each
// new month. The play mode starts it.
//
// The content follows Akhenaten's src/scripts/discord_presence.js (https://github.com/dalerank/Akhenaten, commit
// 8ec540693). Copyright (C) the Akhenaten authors. Licence: GNU Affero General Public License v3.0
// (LICENSES/AGPL-3.0.txt).

#include "game/discord_presence.h"

#include "game/game_data.h"
#include "platform/platform_shell.h"
#include "text/lang_message.h"
#include "text/text_data.h"
#include "window/intermezzo.h"
#include "window/window_data.h"

#include <stdio.h>
#include <string.h>

discord_presence_settings discord_presence_config[DISCORD_PRESENCE_RECORDS] = {
    { "1501648147682693140", 1 },
};

static const char *const egyptian_months[] = {
    "Thoth", "Paopi", "Hathor", "Choiak", "Tybi", "Mekhir", "Phamenoth", "Pharmuthi", "Pachons", "Payni", "Epiphi", "Mesore",
};

enum discord_presence_counts {
    EGYPTIAN_MONTHS = (int32_t) (sizeof(egyptian_months) / sizeof(egyptian_months[0])),
};

static const char *briefing_title(void)
{
    if (settings_data.is_custom_scenario != 0 || !lang_message_data) {
        return "";
    }
    int32_t message_id = mission_briefing_message();
    if (message_id < 0 || message_id >= LANG_MESSAGE_COUNT) {
        return "";
    }
    return (const char *) (lang_message_data + lang_message_get(message_id)->title_text);
}

static void copy_map_name(char *name, size_t size)
{
    snprintf(name, size, "%s", scenario_map_name);
    char *extension = strrchr(name, '.');
    if (extension) {
        *extension = 0;
    }
}

static void copy_mission_title(char *title, size_t size)
{
    snprintf(title, size, "%s", briefing_title());
    if (!title[0]) {
        copy_map_name(title, size);
    }
    if (!title[0]) {
        snprintf(title, size, "%s", settings_data.player_name);
    }
    if (!title[0]) {
        snprintf(title, size, "Tutankhamun");
    }
}

static void copy_date(char *date, size_t size)
{
    int32_t month = game_time_month;
    const char *month_name = month >= 0 && month < EGYPTIAN_MONTHS ? egyptian_months[month] : "";
    int32_t year = game_time_year - scenario_data.start.year + 1;
    snprintf(date, size, "Year %d, %s", year, month_name);
}

static bool presence_wanted(void)
{
    return platform_web_links_allowed() && discord_presence_config[DISCORD_PRESENCE_DISCORD].enabled;
}

void discord_presence_start(void)
{
    if (presence_wanted()) {
        discord_rpc_start(discord_presence_config[DISCORD_PRESENCE_DISCORD].app_id);
    }
}

void discord_presence_update(void)
{
    if (!platform_web_links_allowed() || !discord_rpc_running()) {
        return;
    }
    char title[DISCORD_RPC_TEXT_SIZE];
    char date[DISCORD_RPC_TEXT_SIZE];
    copy_mission_title(title, sizeof(title));
    copy_date(date, sizeof(date));
    discord_rpc_set_activity(title, date);
}

void discord_presence_stop(void)
{
    discord_rpc_stop();
}
