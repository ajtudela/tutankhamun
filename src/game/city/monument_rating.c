// The monument rating, a part of the culture ratings: the weights of the city's monument slots times their progress
// plus a share of the stored goal resources (or, in maximum mode, everything the scenario asks for), as
// 6.32 * sqrt(points) + 0.5, one less while a monument is unfinished. Also the divisor of a burial goal.

#include "city/monument_rating.h"

#include "building/abu_simbel.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_city.h"
#include "building/monument_tables.h"
#include "building/pyramid_blocks.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "city/city_internal.h"
#include "city/kingdom.h"
#include "city/ratings.h"
#include "city/resource_type.h"

#include <math.h>

static int32_t burial_goal_required(int32_t index)
{
    return scenario_data.provisions_needed[index + RESOURCE_MIN];
}

int32_t city_burial_goal_divisor(int32_t resource)
{
    switch (resource) {
        case RESOURCE_GRAIN:
        case RESOURCE_GAMEMEAT:
        case RESOURCE_GEMS:
        case RESOURCE_TIMBER:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
            return BURIAL_GOAL_MATERIAL_DIVISOR;
        case RESOURCE_WEAPONS:
        case RESOURCE_POTTERY:
        case RESOURCE_BEER:
        case RESOURCE_LINEN:
        case RESOURCE_LUXURY_GOODS:
        case RESOURCE_PAPYRUS:
        case RESOURCE_CHARIOTS:
            return BURIAL_GOAL_GOODS_DIVISOR;
        default:
            return 0;
    }
}

static const double MONUMENT_RATING_FACTOR = 6.32;
static const double MONUMENT_RATING_ROUNDING = 0.5;

double city_ratings_calculate_monument(int32_t maximum)
{
    double points = 0;
    int32_t unfinished = 0;
    if (scenario_count_monuments() != 0) {
        if ((uint8_t) maximum) {
            for (int32_t slot = 0; slot < BURIAL_GOAL_MONUMENT_SLOTS; slot++) {
                if (scenario_data.monument_def_ids[slot]) {
                    points += monument_defs[scenario_data.monument_def_ids[slot]].weight;
                }
            }
            for (int32_t i = 0; i < BURIAL_GOAL_RESOURCES; i++) {
                int32_t divisor = city_burial_goal_divisor(i + RESOURCE_MIN);
                int32_t required = burial_goal_required(i);
                if (divisor && required != 0) {
                    points += required / divisor;
                }
            }
        } else {
            const city_player *player = city_player_current();
            for (int32_t slot = 0; slot < BURIAL_GOAL_MONUMENT_SLOTS; slot++) {
                int32_t id = player->monument_building_id[slot];
                if (id == 0) {
                    continue;
                }
                building *b = building_get(id);
                int32_t weight = monument_defs[player->monument_def_id[slot]].weight;
                int32_t progress;
                switch (b->type) {
                    case BUILDING_PYRAMID: {
                        if (b->prev_part_building_id != 0) {
                            continue;
                        }
                        pyramid_progress info;
                        pyramid_get_progress_info(id, &info);
                        if (!pyramid_blocks_finished(id)) {
                            unfinished = 1;
                        }
                        points += calc_adjust_with_percentage(weight, info.percent);
                        continue;
                    }
                    case BUILDING_SMALL_OBELISK:
                    case BUILDING_LARGE_OBELISK:
                        progress = b->data.obelisk.progress;
                        break;
                    case BUILDING_SPHINX:
                        if (player->sphinx_stage < SPHINX_STAGE_COUNT) {
                            unfinished = 1;
                        }
                        points += calc_adjust_with_percentage(weight, (player->sphinx_stage * MONUMENT_PROGRESS_COMPLETE) / SPHINX_STAGE_COUNT);
                        continue;
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
                if (progress < MONUMENT_PROGRESS_COMPLETE) {
                    unfinished = 1;
                }
                points += calc_adjust_with_percentage(weight, progress);
            }
            for (int32_t i = 0; i < BURIAL_GOAL_RESOURCES; i++) {
                int32_t divisor = city_burial_goal_divisor(i + RESOURCE_MIN);
                int32_t required = burial_goal_required(i);
                if (divisor && required != 0) {
                    points += scenario_data.provisions_sent[i + RESOURCE_MIN] / divisor;
                }
            }
        }
    }
    double rating = (sqrt(points) * MONUMENT_RATING_FACTOR) + MONUMENT_RATING_ROUNDING;
    if (unfinished) {
        rating -= 1.0;
        if (rating < 0.0) {
            rating = 0.0;
        }
    }
    return rating;
}
