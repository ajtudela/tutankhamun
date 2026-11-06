// Scenario and city monument queries: the monument counts, the scenario's mausoleum size and extra animation
// script, the completed-monument check of the prosperity, a quarry's image group and the victory score.
//
// The extra script and the mausoleum size look at the first two monument slots only, and the victory score is computed
// in double precision.

#include "building/abu_simbel.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_spots.h"
#include "building/pyramid_images.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "figure/figure_data.h"
#include "game/game_data.h"
#include "map/grid.h"
#include <string.h>

int32_t scenario_count_monuments(void)
{
    int32_t count = 0;
    for (int32_t i = 0; i < MONUMENT_SLOTS; i++) {
        if (scenario_data.monument_def_ids[i] != 0) {
            count++;
        }
    }
    return count;
}

int32_t city_count_monuments(void)
{
    int32_t count = 0;
    for (int32_t i = 0; i < MONUMENT_SLOTS; i++) {
        if (city_player_current()->monument_def_id[i] != 0) {
            count++;
        }
    }
    return count;
}

int32_t scenario_get_extra_script_monument(void)
{
    for (int32_t i = 0; i < 2; i++) {
        int32_t id = scenario_data.monument_def_ids[i];
        if (id != 0 && monument_defs[id].kind == MONUMENT_DEF_KIND_OTHER && monument_defs[id].subkind <= MONUMENT_DEF_MAUSOLEUM) {
            return monument_defs[id].subkind;
        }
    }
    return MONUMENT_DEF_SPHINX;
}

int32_t scenario_get_mausoleum_size(void)
{
    for (int32_t i = 0; i < 2; i++) {
        int32_t id = scenario_data.monument_def_ids[i];
        if (id != 0 && monument_defs[id].kind == MONUMENT_DEF_KIND_OTHER && monument_defs[id].subkind == MONUMENT_DEF_MAUSOLEUM) {
            return monument_defs[id].size;
        }
    }
    return 0;
}

int32_t monument_load_extra_scripts(void)
{
    int32_t monument = MONUMENT_DEF_MAUSOLEUM;
    if (strcmp(scenario_data.name, glory_of_ptolemy_name) != 0) {
        monument = scenario_get_extra_script_monument();
    }
    phrase_weight_block.extra_script_monument = monument;
    if (monument == MONUMENT_DEF_OBELISK) {
        return anim_load_script(str_obelisk_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
    }
    if (monument == MONUMENT_DEF_SUN_TEMPLE) {
        return anim_load_script(str_sun_temple_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
    }
    if (monument != MONUMENT_DEF_MAUSOLEUM) {
        return anim_load_script(str_sphinx_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
    }
    scenario_mausoleum_size_cached = scenario_get_mausoleum_size();
    if (scenario_mausoleum_size_cached == 1) {
        return anim_load_script(str_mausoleum1_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
    }
    if (scenario_mausoleum_size_cached == 2) {
        return anim_load_script(str_mausoleum2_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
    }
    return anim_load_script(str_mausoleum0_extra, MONUMENT_SCRIPT_SLOT_EXTRA);
}

int32_t city_has_completed_monument(void)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type == BUILDING_PYRAMID) {
            if (b->monument_remaining_blocks < 1) {
                int32_t offset = pyramid_complex_temple_offset[b->data.pyramid.size_variant] + b->grid_offset - map_info.grid_start;
                pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_FIND_FREE_TILE, 0);
                if (pyramid_complex_free_tile == 0) {
                    return 1;
                }
            }
        } else if (b->type == BUILDING_SPHINX && city_player_current()->sphinx_stage >= SPHINX_STAGE_COUNT) {
            return 1;
        }
    }
    return 0;
}

int32_t quarry_image_group(int32_t resource)
{
    return (resource == RESOURCE_LIMESTONE) ? IMAGE_GROUP_LIMESTONE_QUARRY : IMAGE_GROUP_GRANITE_QUARRY;
}

int32_t city_victory_score(int32_t months)
{
    const city_player *player = city_player_current();
    int32_t weight = 0;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        int32_t id = player->monument_building_id[slot];
        if (id <= 0) {
            continue;
        }
        building *b = building_get(id);
        int32_t progress;
        switch (b->type) {
            case BUILDING_PYRAMID: {
                pyramid_progress info;
                pyramid_get_progress_info(id, &info);
                progress = info.percent;
                break;
            }
            case BUILDING_SMALL_OBELISK:
            case BUILDING_LARGE_OBELISK:
                progress = b->data.obelisk.progress;
                break;
            case BUILDING_SPHINX:
                progress = (player->sphinx_stage * MONUMENT_PROGRESS_COMPLETE) / SPHINX_STAGE_COUNT;
                break;
            case BUILDING_SUN_TEMPLE:
                progress = sun_temple_get_progress(id, 0);
                break;
            case BUILDING_MAUSOLEUM:
                progress = mausoleum_get_progress(id, 0);
                break;
            case BUILDING_ALEXANDRIA_LIBRARY:
                progress = library_get_progress(&library_object, id, 0);
                break;
            case BUILDING_CAESAREUM:
                progress = caesareum_get_progress(&caesareum_object, id, 0);
                break;
            case BUILDING_LIGHTHOUSE:
                progress = lighthouse_get_progress(&lighthouse_object, id, 0);
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
                progress = royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, id, 0);
                break;
            case BUILDING_ABU_SIMBEL:
                progress = abu_simbel_get_progress(&abu_simbel_object, id, 0);
                break;
            case BUILDING_MEDIUM_ROYAL_TOMB:
                progress = royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, id, 0);
                break;
            case BUILDING_LARGE_ROYAL_TOMB:
                progress = royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, id, 0);
                break;
            case BUILDING_GRAND_ROYAL_TOMB:
                progress = royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, id, 0);
                break;
            default:
                continue;
        }
        if (progress == MONUMENT_PROGRESS_COMPLETE) {
            weight += monument_defs[player->monument_def_id[slot]].weight;
        }
    }
    int32_t ratings = player->rating_kingdom + player->rating_prosperity + player->rating_culture;
    double rating_part = (double) ratings * ((double) player->population * 0.002);
    double monument_part = ((double) weight / ((double) (months * months) * 0.00015625)) * ((double) weight * 20.0);
    double score = monument_part + rating_part;
    score = score + (double) player->treasury / ((double) months * 0.03333333333333333);
    score = score * (((double) player->difficulty + 1.0) * 0.3333333333333333);
    return (int32_t) (int64_t) score - player->cheats_used;
}
