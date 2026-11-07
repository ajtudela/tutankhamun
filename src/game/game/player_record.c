// The player's record: its path (save\<player name>.dat), writing it, and recording a won mission from the current
// city into the mission's entry before saving.

#include "game/player_record.h"

#include "city/city_player.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "platform/platform_file.h"
#include "scenario/scenario_data.h"
#include "window/window_data.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const char *player_record_path(void)
{
    char *path = player_record_path_buffer;
    strcpy(path, save_folder_path);
    strcat(path, settings_data.player_name);
    strcat(path, player_file_extension);
    return path;
}

bool player_data_write_file(const void *self, const char *file)
{
    FILE *f = platform_fopen(file, file_mode_write_binary);
    if (!f) {
        return 0;
    }
    size_t written = fwrite(self, sizeof(player_data), 1, f);
    fclose(f);
    return written == 1;
}

int32_t player_record_mission(int32_t mission, int32_t monuments, int32_t score, int32_t months)
{
    const city_player *player = city_player_current();
    mission_record *record = &campaign_object.campaign.mission_records[mission];
    record->rating_culture = player->rating_culture;
    record->difficulty = player->difficulty;
    record->treasury = player->treasury;
    record->population = player->population;
    record->rating_kingdom = player->rating_kingdom;
    record->mission = mission;
    record->used = 1;
    record->monuments = monuments;
    strcpy(record->player_name, settings_data.player_name);
    record->rating_prosperity = player->rating_prosperity;
    record->score = score;
    record->months = months;
    return player_data_write_file(&campaign_object, player_record_path());
}
