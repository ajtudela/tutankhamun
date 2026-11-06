// The locust plague, checked once a month: two months before the flood, once its countdown has run out, every farm gets
// nine locusts and loses its workers, and the locust sound replaces the music.

#include "city/locusts.h"

#include "building/building_data.h"
#include "building/building_industry.h"
#include "city/city_data.h"
#include "city/city_internal.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "flood/flood_internal.h"
#include "game/game_data.h"
#include "map/map_data.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"

void city_locusts_update(void)
{
    if (city_locusts_countdown != LOCUST_COUNTDOWN_RUN_OUT) {
        return;
    }
    int32_t flood_month = floods_get_month_before(&floods_data);
    if (flood_month != game_time_month + LOCUST_MONTHS_BEFORE_FLOOD && flood_month != game_time_month + LOCUST_MONTHS_BEFORE_FLOOD - MONTHS_PER_YEAR) {
        return;
    }
    city_locusts_countdown = LOCUST_COUNTDOWN_AFTER_PLAGUE;
    for (int32_t farm_id = 1; farm_id <= building_highest_id; farm_id++) {
        if (!building_is_farm(farm_id)) {
            continue;
        }
        building *farm = building_get(farm_id);
        for (int32_t i = 0; i < LOCUSTS_PER_FARM; i++) {
            random_generate_next();
            int32_t offset = random_generator_state.random1_15bit % (map_info.grid_width / 4);
            int32_t locust_id = figure_create(city_current_player, FIGURE_LOCUST, ((map_info.grid_width * 7) / 8) - offset, ((map_info.grid_width * 3) / 8) - offset, 0);
            farm->immigrant_figure_id = (int16_t) locust_id;
            figure *locust = figure_get(locust_id);
            locust->action_state = FIGURE_ACTION_LOCUST_CREATED;
            locust->immigrant_building_id = (int16_t) farm_id;
            random_generate_next();
            locust->wait_ticks = (int16_t) (random_generator_state.random1_15bit % LOCUST_MAX_START_DELAY);
            locust->migrant_num_people = 0;
            locust->terrain_usage = TERRAIN_USAGE_ANYWHERE;
            locust->destination_x = (int16_t) ((i / LOCUST_SQUARE_SIDE) + farm->x);
            locust->destination_y = (int16_t) ((i % LOCUST_SQUARE_SIDE) + farm->y);
        }
        if (farm->num_workers > 0) {
            farm->data.industry.labor_days_left[0] = 0;
            farm->num_workers = 0;
            int32_t laborer_id = farm->data.industry.worker_figure_id;
            if (laborer_id != 0) {
                figure *laborer = figure_get(laborer_id);
                if (laborer->state != FIGURE_STATE_DEAD && laborer->type == FIGURE_LABORER) {
                    laborer->state = FIGURE_STATE_DEAD;
                    map_figure_delete(laborer_id);
                }
                farm->data.industry.worker_figure_id = 0;
            }
        }
    }
    if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_MUSIC)) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_MUSIC);
    }
    sound_music_playing_special = 1;
    if (sound_lib_stream_is_playing(&sound_manager, SOUND_STREAM_CITY)) {
        sound_lib_stop_stream(&sound_manager, SOUND_STREAM_CITY);
    }
    sound_lib_play_stream(&sound_manager, locust_sound_file, SOUND_STREAM_CITY, 1);
}
