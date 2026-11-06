// The monument framework: the city's monument slots, the monument definitions and the resources the monuments
// and guilds need; and the pyramid's progress report, part creation, entry tile and complex temple.
//
// The definition lookups have two fall-throughs:
// - a mausoleum without a matching definition falls through to the small royal tomb's;
// - any definition of subkind 4 counts as a pyramid, so a mausoleum slot is reported as a pyramid.

#include "building/monument.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_load.h"
#include "building/construction_place.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument_checks.h"
#include "building/monument_city.h"
#include "building/monument_internal.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/obelisk.h"
#include "building/pyramid.h"
#include "building/pyramid_blocks.h"
#include "building/pyramid_images.h"
#include "building/pyramid_level.h"
#include "building/pyramid_parts.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "game/undo_build.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "platform/log.h"
#include <string.h>

static int32_t part_layers(int32_t size, int32_t index)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return pyramid_layout.mastaba[size][index].layers;
    }
    return pyramid_layout.pyramid[size][index].layers;
}

static int32_t entry_part_offset(int32_t size)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return mastaba_entry.entry_points[size][0].part;
    }
    return pyramid_ramp_table[size][0].part;
}

void city_set_monument_building_id(int32_t building_id, int32_t slot)
{
    city_player_current()->monument_building_id[slot] = building_id;
}

void city_clear_monument_building_id(int32_t building_id, int32_t slot)
{
    (void) building_id;
    city_player_current()->monument_building_id[slot] = 0;
}

int32_t city_find_monument_slot(int32_t building_id)
{
    int32_t slot = -1;
    for (int32_t i = 0; i < MONUMENT_SLOTS && slot < 0; i++) {
        if (city_player_current()->monument_building_id[i] == building_id) {
            slot = i;
        }
    }
    return slot;
}

void monument_unregister(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->prev_part_building_id != 0) {
        return;
    }
    city_player *player = city_player_get(city_current_player);
    int32_t slot = 0;
    while (player->monument_building_id[slot] != building_id) {
        if (++slot >= MONUMENT_SLOTS) {
            return;
        }
    }
    player->monument_def_id[slot] = 0;
    if (b->type == BUILDING_SMALL_OBELISK || b->type == BUILDING_LARGE_OBELISK) {
        obelisk_load_script_for_slot(slot);
    }
    city_clear_monument_building_id(building_id, slot);
}

static int32_t find_def(int32_t kind, int32_t subkind)
{
    for (int32_t i = 0; i < MONUMENT_DEFINITIONS; i++) {
        if (monument_defs[i].kind == kind && monument_defs[i].subkind == subkind) {
            return i;
        }
    }
    return -1;
}

static int32_t find_def_sized(int32_t kind, int32_t subkind, int32_t size)
{
    for (int32_t i = 0; i < MONUMENT_DEFINITIONS; i++) {
        if (monument_defs[i].kind == kind && monument_defs[i].subkind == subkind && monument_defs[i].size == size) {
            return i;
        }
    }
    return -1;
}

int32_t monument_get_def_id(int32_t building_id)
{
    building *b = building_get(building_id);
    switch (b->type) {
        case BUILDING_PYRAMID:
            for (int32_t i = 0; i < MONUMENT_DEFINITIONS; i++) {
                const monument_def *def = &monument_defs[i];
                if (def->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY && def->subkind == scenario_get_pyramid_style() &&
                    def->size == b->data.pyramid.size_variant) {
                    return i;
                }
            }
            return -1;
        case BUILDING_SMALL_OBELISK:
            return find_def_sized(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_OBELISK, 0);
        case BUILDING_LARGE_OBELISK:
            return find_def_sized(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_OBELISK, 1);
        case BUILDING_SPHINX:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_SPHINX);
        case BUILDING_SUN_TEMPLE:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_SUN_TEMPLE);
        case BUILDING_MAUSOLEUM:
            for (int32_t i = 0; i < MONUMENT_DEFINITIONS; i++) {
                const monument_def *def = &monument_defs[i];
                if (def->kind == MONUMENT_DEF_KIND_OTHER && def->subkind == MONUMENT_DEF_MAUSOLEUM && def->size == scenario_get_mausoleum_size()) {
                    return i;
                }
            }
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_SMALL_ROYAL_TOMB);
        case BUILDING_SMALL_ROYAL_TOMB:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_SMALL_ROYAL_TOMB);
        case BUILDING_ALEXANDRIA_LIBRARY:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_LIBRARY);
        case BUILDING_CAESAREUM:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_CAESAREUM);
        case BUILDING_LIGHTHOUSE:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_LIGHTHOUSE);
        case BUILDING_ABU_SIMBEL:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_ABU_SIMBEL);
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_MEDIUM_ROYAL_TOMB);
        case BUILDING_LARGE_ROYAL_TOMB:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_LARGE_ROYAL_TOMB);
        case BUILDING_GRAND_ROYAL_TOMB:
            return find_def(MONUMENT_DEF_KIND_OTHER, MONUMENT_DEF_GRAND_ROYAL_TOMB);
        default:
            return -1;
    }
}

int32_t monument_register(int32_t building_id)
{
    if (building_get(building_id)->prev_part_building_id != 0) {
        return -1;
    }
    int32_t def_id = monument_get_def_id(building_id);
    if (def_id < 0) {
        return -1;
    }
    city_player *player = city_player_get(1);
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        if (scenario_data.monument_def_ids[slot] == def_id && player->monument_def_id[slot] == 0) {
            player->monument_def_id[slot] = def_id;
            city_set_monument_building_id(building_id, slot);
            return slot;
        }
    }
    return -1;
}

int32_t scenario_monument_slot_obelisk_type(int32_t slot)
{
    const monument_def *def = &monument_defs[scenario_data.monument_def_ids[slot]];
    if (def->kind != MONUMENT_DEF_KIND_OTHER || def->subkind != MONUMENT_DEF_OBELISK) {
        return 0;
    }
    return (def->size != 0) ? BUILDING_LARGE_OBELISK : BUILDING_SMALL_OBELISK;
}

int32_t scenario_monument_slot_is_pyramid(int32_t slot)
{
    const monument_def *def = &monument_defs[scenario_data.monument_def_ids[slot]];
    if (def->kind == MONUMENT_DEF_KIND_PYRAMID_FAMILY && def->subkind >= PYRAMID_STYLE_BENT && def->subkind <= PYRAMID_STYLE_MASTABA) {
        return 1;
    }
    return def->subkind == MONUMENT_DEF_MAUSOLEUM;
}

int32_t monument_def_uses_resource(int32_t def_id, int32_t resource)
{
    const monument_def *def = &monument_defs[def_id];
    return def->resources[0] == resource || def->resources[1] == resource || def->resources[2] == resource;
}

static int32_t object_progress(int32_t building_id, int32_t type)
{
    switch (type) {
        case BUILDING_SUN_TEMPLE:
            return sun_temple_get_progress(building_id, 0);
        case BUILDING_ALEXANDRIA_LIBRARY:
            return library_get_progress(&library_object, building_id, 0);
        case BUILDING_ABU_SIMBEL:
            return abu_simbel_get_progress(&abu_simbel_object, building_id, 0);
        case BUILDING_CAESAREUM:
            return caesareum_get_progress(&caesareum_object, building_id, 0);
        case BUILDING_SMALL_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, building_id, 0);
        case BUILDING_MEDIUM_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, building_id, 0);
        case BUILDING_LARGE_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, building_id, 0);
        case BUILDING_GRAND_ROYAL_TOMB:
            return royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, building_id, 0);
        case BUILDING_LIGHTHOUSE:
            return lighthouse_get_progress(&lighthouse_object, building_id, 0);
        default:
            return mausoleum_get_progress(building_id, 0);
    }
}

bool city_monuments_need_resource(int32_t resource, bool force)
{
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        int32_t def_id = city_player_current()->monument_def_id[slot];
        if (def_id == 0) {
            if (monument_def_uses_resource(scenario_data.monument_def_ids[slot], resource)) {
                return 1;
            }
            continue;
        }
        int32_t building_id = city_player_current()->monument_building_id[slot];
        if (building_id == 0) {
            continue;
        }
        building *b = building_get(building_id);
        int32_t incomplete;
        switch (b->type) {
            case BUILDING_PYRAMID: {
                pyramid_progress progress;
                pyramid_get_progress_info(building_id, &progress);
                incomplete = progress.percent < MONUMENT_PROGRESS_COMPLETE;
                break;
            }
            case BUILDING_SPHINX:
                incomplete = city_player_current()->sphinx_stage < SPHINX_STAGE_COUNT;
                break;
            case BUILDING_SMALL_OBELISK:
            case BUILDING_LARGE_OBELISK:
                incomplete = b->data.obelisk.progress != MONUMENT_PROGRESS_COMPLETE;
                break;
            default:
                incomplete = object_progress(building_id, b->type) < MONUMENT_PROGRESS_COMPLETE;
                break;
        }
        if ((incomplete || force) && monument_def_uses_resource(def_id, resource)) {
            return 1;
        }
    }
    return 0;
}

int32_t guild_has_monument_demand(int32_t guild_type)
{
    switch (guild_type) {
        case BUILDING_CARPENTERS_GUILD:
            return city_monuments_need_resource(RESOURCE_TIMBER, 0);
        case BUILDING_BRICKLAYERS_GUILD:
            return city_monuments_need_resource(RESOURCE_BRICKS, 0);
        case BUILDING_STONEMASONS_GUILD:
            return city_monuments_need_resource(RESOURCE_STONE, 0) || city_monuments_need_resource(RESOURCE_LIMESTONE, 0) ||
                city_monuments_need_resource(RESOURCE_GRANITE, 0) || city_monuments_need_resource(RESOURCE_SANDSTONE, 0) ||
                city_monuments_need_resource(RESOURCE_LAMPS, 0) || city_monuments_need_resource(RESOURCE_MARBLE, 0);
        case BUILDING_ARTISANS_GUILD:
            return city_monuments_need_resource(RESOURCE_PAINT, 0) || city_monuments_need_resource(RESOURCE_CLAY, 0);
        default:
            return 0;
    }
}

int32_t guild_serves_monument_def(int32_t guild_type, int32_t def_id)
{
    switch (guild_type) {
        case BUILDING_CARPENTERS_GUILD:
            return monument_def_uses_resource(def_id, RESOURCE_TIMBER);
        case BUILDING_BRICKLAYERS_GUILD:
            return monument_def_uses_resource(def_id, RESOURCE_BRICKS);
        case BUILDING_STONEMASONS_GUILD:
            return monument_def_uses_resource(def_id, RESOURCE_STONE) || monument_def_uses_resource(def_id, RESOURCE_LIMESTONE) ||
                monument_def_uses_resource(def_id, RESOURCE_GRANITE) || monument_def_uses_resource(def_id, RESOURCE_SANDSTONE) ||
                monument_def_uses_resource(def_id, RESOURCE_LAMPS) || monument_def_uses_resource(def_id, RESOURCE_MARBLE);
        case BUILDING_ARTISANS_GUILD:
            return monument_def_uses_resource(def_id, RESOURCE_PAINT) || monument_def_uses_resource(def_id, RESOURCE_CLAY);
        default:
            return 0;
    }
}

int32_t monument_needs_resource(int32_t building_id, int32_t resource)
{
    switch (building_get(building_id)->type) {
        case BUILDING_PYRAMID: {
            pyramid_progress p;
            pyramid_get_progress_info(building_id, &p);
            switch (scenario_pyramid_style) {
                case PYRAMID_STYLE_BENT:
                case PYRAMID_STYLE_PYRAMID:
                    if (resource == RESOURCE_LIMESTONE) {
                        return p.casing_remaining > 0;
                    }
                    return resource == RESOURCE_STONE && p.main_remaining > 0;
                case PYRAMID_STYLE_MUDBRICK:
                    if (resource == RESOURCE_LIMESTONE) {
                        return p.casing_remaining > 0;
                    }
                    return resource == RESOURCE_BRICKS && p.main_remaining > 0;
                case PYRAMID_STYLE_STEPPED:
                    return resource == RESOURCE_STONE && (p.main_remaining > 0 || p.casing_remaining > 0);
                case PYRAMID_STYLE_MASTABA:
                    return resource == RESOURCE_BRICKS && (p.main_remaining > 0 || p.casing_remaining > 0);
                default:
                    return 0;
            }
        }
        case BUILDING_SUN_TEMPLE: {
            int32_t out[SUN_TEMPLE_PROGRESS_SLOTS];
            if (resource != RESOURCE_SANDSTONE) {
                return 0;
            }
            sun_temple_get_progress(building_id, out);
            return out[SUN_TEMPLE_PROGRESS_MATERIAL] != 0;
        }
        case BUILDING_MAUSOLEUM: {
            mausoleum_info_progress out;
            if (resource != RESOURCE_SANDSTONE) {
                return 0;
            }
            mausoleum_get_progress(building_id, &out);
            return out.material != 0;
        }
        default:
            return 0;
    }
}

void city_rebuild_monument_slots(void)
{
    city_player *player = city_player_get(city_current_player);
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        player->monument_def_id[slot] = 0;
        player->monument_building_id[slot] = 0;
    }
    for (int32_t i = 1; i <= building_highest_id; i++) {
        building *b = building_get(i);
        if ((b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_CREATED) || b->created_sequence == MONUMENT_KEPT_FROM_EARLIER_MISSION) {
            continue;
        }
        int32_t building_id = i;
        int32_t def_id;
        switch (b->type) {
            case BUILDING_PYRAMID:
                def_id = monument_get_def_id(building_id);
                i += pyramid_part_count_of_size(b->data.pyramid.size_variant) - 1;
                break;
            case BUILDING_SUN_TEMPLE:
                def_id = monument_get_def_id(building_id);
                i += SUN_TEMPLE_PARTS - 1;
                break;
            case BUILDING_CAESAREUM:
                def_id = monument_get_def_id(building_id);
                i += CAESAREUM_PARTS - 1;
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
                def_id = monument_get_def_id(building_id);
                i += SMALL_TOMB_PARTS - 1;
                break;
            case BUILDING_MEDIUM_ROYAL_TOMB:
                def_id = monument_get_def_id(building_id);
                i += MEDIUM_TOMB_PARTS - 1;
                break;
            case BUILDING_LARGE_ROYAL_TOMB:
                def_id = monument_get_def_id(building_id);
                i += LARGE_TOMB_PARTS - 1;
                break;
            case BUILDING_GRAND_ROYAL_TOMB:
                def_id = monument_get_def_id(building_id);
                i += GRAND_TOMB_PARTS - 1;
                break;
            case BUILDING_ALEXANDRIA_LIBRARY:
                def_id = monument_get_def_id(building_id);
                i += LIBRARY_PARTS - 1;
                break;
            case BUILDING_ABU_SIMBEL:
                def_id = monument_get_def_id(building_id);
                i += ABU_SIMBEL_PARTS - 1;
                break;
            case BUILDING_MAUSOLEUM:
                def_id = monument_get_def_id(building_id);
                i += MAUSOLEUM_PARTS - 1;
                break;
            case BUILDING_SPHINX:
                if (b->prev_part_building_id != 0) {
                    continue;
                }
                def_id = monument_get_def_id(building_id);
                break;
            case BUILDING_SMALL_OBELISK:
            case BUILDING_LARGE_OBELISK:
            case BUILDING_LIGHTHOUSE:
                def_id = monument_get_def_id(building_id);
                break;
            default:
                continue;
        }
        if (def_id <= 0) {
            continue;
        }
        for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
            if (def_id == scenario_data.monument_def_ids[slot] && player->monument_building_id[slot] == 0) {
                player->monument_def_id[slot] = def_id;
                player->monument_building_id[slot] = building_id;
                break;
            }
        }
    }
}

static const int32_t prep_mask_weight[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

static int32_t block_total(int32_t size)
{
    switch (scenario_pyramid_style) {
        case PYRAMID_STYLE_BENT:
            return bent_pyramid_block_total[size];
        case PYRAMID_STYLE_MUDBRICK:
        case PYRAMID_STYLE_PYRAMID:
            return pyramid_block_total[size];
        case PYRAMID_STYLE_STEPPED:
            return stepped_pyramid_block_total[size];
        case PYRAMID_STYLE_MASTABA:
            return mastaba_block_total[size];
        default:
            return 0;
    }
}

static int32_t casing_total(int32_t size)
{
    switch (scenario_pyramid_style) {
        case PYRAMID_STYLE_BENT:
            return bent_pyramid_casing_total[size];
        case PYRAMID_STYLE_MUDBRICK:
        case PYRAMID_STYLE_PYRAMID:
            return pyramid_part_count_of_size(size) * PYRAMID_BLOCKS_PER_LAYER;
        default:
            return 0;
    }
}

static int32_t bent_layer_units(int32_t size, int32_t layer)
{
    return pyramid_layout.bent_half_layer[(size * BENT_PYRAMID_LAYER_STRIDE) + layer] ? 3 : 6;
}

static void add_part_totals(pyramid_progress *out, int32_t size, int32_t index, const building *part, int32_t layers, int32_t casing)
{
    int32_t style = scenario_pyramid_style;
    if (style == PYRAMID_STYLE_MASTABA && index == 0) {
        out->main_total += 1;
    } else if (style == PYRAMID_STYLE_BENT) {
        for (int32_t i = 0; i < layers; i++) {
            out->main_total += bent_layer_units(size, i);
        }
        out->casing_total += bent_layer_units(size, part->data.pyramid.target_layers);
    } else {
        out->main_total += layers * PYRAMID_BLOCKS_PER_LAYER;
        if (casing != 0) {
            out->casing_total += PYRAMID_BLOCKS_PER_LAYER;
        } else {
            out->main_total += PYRAMID_BLOCKS_PER_LAYER;
        }
    }
}

static void add_part_placed(pyramid_progress *out, int32_t size, int32_t index, const building *part, int32_t layers, int32_t casing)
{
    int32_t style = scenario_pyramid_style;
    const building_pyramid *p = &part->data.pyramid;
    if (p->stage == 0) {
        int32_t done = (p->layer < layers) ? p->layer : layers;
        if (done < 0) {
            done = 0;
        }
        if (style == PYRAMID_STYLE_BENT) {
            for (int32_t i = 0; i < done; i++) {
                out->main_placed += bent_layer_units(size, i);
            }
        } else if (style != PYRAMID_STYLE_MASTABA || index != 0) {
            out->main_placed += done * PYRAMID_BLOCKS_PER_LAYER;
        }
        if (p->layer < p->target_layers || casing == 0) {
            out->main_placed += p->blocks;
        } else {
            out->casing_placed += p->blocks;
        }
    } else if (style == PYRAMID_STYLE_BENT) {
        for (int32_t i = 0; i < layers; i++) {
            out->main_placed += bent_layer_units(size, i);
        }
        out->casing_placed += bent_layer_units(size, p->layer);
    } else {
        out->main_placed += style == PYRAMID_STYLE_MASTABA && index == 0 ? 1 : layers * PYRAMID_BLOCKS_PER_LAYER;
        if (casing != 0) {
            out->casing_placed += PYRAMID_BLOCKS_PER_LAYER;
        } else if (style != PYRAMID_STYLE_MASTABA || index != 0) {
            out->main_placed += PYRAMID_BLOCKS_PER_LAYER;
        }
    }
}

static void count_waiting_worker(pyramid_progress *out, int32_t part_id, const figure *f, int32_t worker_id, int32_t casing)
{
    int32_t laborer_id = f->leading_figure_id;
    if (laborer_id != 0 && pyramid_laborer_brings_block_to(laborer_id, worker_id)) {
        return;
    }
    out->waiting++;
    if (!pyramid_part_needs_main_material(part_id) && casing != 0) {
        out->waiting_casing++;
    } else {
        out->waiting_main++;
    }
}

static void count_part_worker(pyramid_progress *out, int32_t part_id, int32_t casing)
{
    int32_t worker_id = building_get(part_id)->data.pyramid.worker_figure_id;
    if (worker_id == 0) {
        return;
    }
    const figure *f = figure_get(worker_id);
    switch (f->type) {
        case FIGURE_CARPENTER:
            if (pyramid_part_has_active_worker(part_id)) {
                out->carpenters++;
            }
            break;
        case FIGURE_BRICKLAYER:
            if (pyramid_part_has_active_worker(part_id)) {
                out->bricklayers++;
                out->block_workers++;
                if (f->action_state == FIGURE_ACTION_BRICKLAYER_WAIT_FOR_BLOCK) {
                    count_waiting_worker(out, part_id, f, worker_id, casing);
                }
            }
            break;
        case FIGURE_STONEMASON:
            if (pyramid_part_has_active_worker(part_id)) {
                out->stonemasons++;
                out->block_workers++;
                if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    count_waiting_worker(out, part_id, f, worker_id, casing);
                }
            }
            break;
        case FIGURE_LABORER:
            if (pyramid_part_has_active_worker(part_id)) {
                out->laborers++;
            }
            break;
    }
}

void pyramid_get_progress_info(int32_t building_id, pyramid_progress *out)
{
    memset(out, 0, sizeof(*out));
    if (building_get(building_id)->type != BUILDING_PYRAMID) {
        return;
    }
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    int32_t style = scenario_pyramid_style;
    int32_t prep_total = 0;
    if (style >= PYRAMID_STYLE_BENT && style <= PYRAMID_STYLE_PYRAMID) {
        prep_total = pyramid_prep_work_total[size];
    } else if (style == PYRAMID_STYLE_MASTABA) {
        prep_total = mastaba_prep_work_total[size];
    }
    out->prep_total = prep_total;
    int32_t blocks = block_total(size);
    int32_t casing = casing_total(size);

    if (main->data.pyramid.stage > PYRAMID_STAGE_CASING) {
        out->phase = PYRAMID_PHASE_SITE_PREPARATION;
        out->prep_max_stage = 0;
        out->prep_min_stage = PYRAMID_PREP_MIN_STAGE_NONE;
        out->prep_done = 0;
        for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
            building *part = building_get(main_id + i);
            int32_t stage = part->data.pyramid.stage;
            if (stage > out->prep_max_stage) {
                out->prep_max_stage = stage;
            }
            if (stage < out->prep_min_stage) {
                out->prep_min_stage = stage;
            }
            int32_t worker_id = part->data.pyramid.worker_figure_id;
            if (worker_id != 0 && figure_get(worker_id)->type == FIGURE_LABORER && pyramid_part_has_active_worker(main_id + i)) {
                out->laborers++;
            }
            int32_t quadrants = prep_mask_weight[part->data.pyramid.prep_mask];
            switch (stage) {
                case PYRAMID_STAGE_LEVELED:
                    out->prep_done += quadrants;
                    break;
                case PYRAMID_STAGE_LEVEL_1:
                    out->prep_done += 5;
                    break;
                case PYRAMID_STAGE_LEVEL_2:
                    out->prep_done += quadrants + 5;
                    break;
                case PYRAMID_STAGE_LEVEL_3:
                    out->prep_done += quadrants + 9;
                    break;
                case PYRAMID_STAGE_LEVEL_4:
                    out->prep_done += quadrants + 13;
                    break;
                case PYRAMID_STAGE_LEVEL_5:
                    out->prep_done += quadrants + 17;
                    break;
                case PYRAMID_STAGE_LEVEL_6:
                    out->prep_done += quadrants + 21;
                    break;
            }
        }
        if (out->prep_max_stage == PYRAMID_STAGE_LEVEL_TOP) {
            out->prep_done++;
        }
        out->phase_percent = (out->prep_done * MONUMENT_PROGRESS_COMPLETE) / out->prep_total;
        out->percent = (out->phase_percent * PYRAMID_PREPARATION_PERCENT) / MONUMENT_PROGRESS_COMPLETE;
        for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
            add_part_totals(out, size, i, building_get(main_id + i), part_layers(size, i), casing);
        }
    } else {
        out->prep_done = prep_total;
        pyramid_find_frontier_parts(main_id);
        int32_t first_id = (pyramid_frontier_count > 0) ? pyramid_frontier_parts[0] : main_id;
        const building *first = building_get(first_id);
        out->height = first->data.pyramid.blocks + first->data.pyramid.layer * PYRAMID_BLOCKS_PER_LAYER;
        out->height_next = out->height + 1;
        if (scenario_pyramid_style == PYRAMID_STYLE_BENT) {
            out->layer_count = bent_pyramid_layer_count[size];
        } else if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
            out->layer_count = mastaba_layer_count[size];
        } else {
            out->layer_count = pyramid_layer_count[size];
        }
        if (pyramid_frontier_count < 1) {
            out->phase = PYRAMID_PHASE_COMPLETE;
            out->phase_percent = MONUMENT_PROGRESS_COMPLETE;
            out->percent = MONUMENT_PROGRESS_COMPLETE;
        } else {
            int32_t remaining = main->monument_remaining_blocks;
            if (first->data.pyramid.stage == PYRAMID_STAGE_MAIN_MATERIAL) {
                out->phase = PYRAMID_PHASE_BLOCKS;
                out->phase_percent = ((blocks - remaining) * MONUMENT_PROGRESS_COMPLETE) / (blocks - casing);
            } else {
                out->phase = PYRAMID_PHASE_FINISHING;
                out->phase_percent = ((casing - remaining) * MONUMENT_PROGRESS_COMPLETE) / casing;
            }
            out->percent = (((blocks - remaining) * (MONUMENT_PROGRESS_COMPLETE - PYRAMID_PREPARATION_PERCENT)) / blocks) + PYRAMID_PREPARATION_PERCENT;
        }
        for (int32_t i = 0; i < pyramid_part_count_of_size(size); i++) {
            int32_t layers = part_layers(size, i);
            const building *part = building_get(main_id + i);
            add_part_placed(out, size, i, part, layers, casing);
            add_part_totals(out, size, i, part, layers, casing);
            count_part_worker(out, main_id + i, casing);
        }
        if (out->phase == PYRAMID_PHASE_BLOCKS) {
            for (int32_t i = 0; i < pyramid_frontier_count; i++) {
                if (!pyramid_part_needs_main_material(pyramid_frontier_parts[i]) && casing != 0) {
                    out->frontier_casing++;
                } else {
                    out->frontier_main++;
                }
            }
        }
    }
    out->main_remaining = (out->main_total - out->main_placed) / PYRAMID_UNITS_PER_BLOCK;
    out->casing_remaining = (out->casing_total - out->casing_placed) / PYRAMID_UNITS_PER_BLOCK;
    out->main_total /= PYRAMID_UNITS_PER_BLOCK;
    out->casing_total /= PYRAMID_UNITS_PER_BLOCK;
    out->main_placed /= PYRAMID_UNITS_PER_BLOCK;
    out->casing_placed /= PYRAMID_UNITS_PER_BLOCK;
    out->frontier_main /= PYRAMID_UNITS_PER_BLOCK;
    out->frontier_casing /= PYRAMID_UNITS_PER_BLOCK;
    out->material_main = out->waiting_main * PYRAMID_MATERIAL_PER_WAITING;
    out->material_casing = out->waiting_casing * PYRAMID_MATERIAL_PER_WAITING;
}

void pyramid_part_mark_terrain(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t x = b->x;
    int32_t y = b->y;
    int32_t size = b->size;
    if (x < 0 || y < 0 || x + size - 1 >= map_info.grid_width || y + size - 1 >= map_info.grid_height) {
        return;
    }
    int32_t base = map_info.grid_start + y * GRID_SIZE + x;
    map_tile_cursor_offset = base;
    for (int32_t row = 0; row < size; row++) {
        for (int32_t col = 0; col < size; col++) {
            int32_t offset = base + building_square_tile_offset(row, col);
            map_grids.terrain[offset] |= TERRAIN_MONUMENT | TERRAIN_BUILDING;
            map_grids.building_ids[offset] = (int16_t) building_id;
        }
    }
}

static void init_part(building *b, int32_t size, int32_t index)
{
    b->data.pyramid.size_variant = (uint8_t) size;
    b->data.pyramid.stage = PYRAMID_STAGE_SITE_PREPARATION;
    b->data.pyramid.blocks = 0;
    b->data.pyramid.worker_figure_id = 0;
    b->data.pyramid.layer = 0;
    b->data.pyramid.target_layers = (uint8_t) part_layers(size, index);
    b->data.pyramid.ramp1_height = 0;
    b->data.pyramid.ramp2_height = 0;
    b->data.pyramid.prep_mask = PYRAMID_PREP_ALL_QUADRANTS;
    b->data.pyramid.ramp_image_x1 = 0;
    b->data.pyramid.ramp_image_y1 = 0;
    b->data.pyramid.ramp_image_x2 = 0;
    b->data.pyramid.ramp_image_y2 = 0;
    b->data.pyramid.ramp_image_id = 0;
}

void pyramid_create_parts(int32_t player, int32_t x, int32_t y, int32_t building_id)
{
    building *main = building_get(building_id);
    int32_t size = pyramid_layout.size_being_placed;
    main->prev_part_building_id = 0;
    init_part(main, size, 0);
    main->monument_remaining_blocks = (int16_t) block_total(size);
    main->subtype = 0;
    main->size = PYRAMID_PART_SIZE;
    pyramid_part_mark_terrain(building_id);
    monument_register(building_id);

    int32_t prev_id = building_id;
    for (int32_t i = 1; i < pyramid_part_count_of_size(pyramid_layout.size_being_placed); i++) {
        const pyramid_tile_offset *offset = (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? &mastaba_part_xy[i] : &pyramid_part_offsets[i - 1];
        int32_t part_id = building_create(player, BUILDING_PYRAMID, x + offset->x, y + offset->y, 0, prev_id + 1);
        game_undo_add_building(part_id);
        building *part = building_get(part_id);
        part->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) part_id;
        part->next_part_building_id = 0;
        part->subtype = (int16_t) i;
        part->size = PYRAMID_PART_SIZE;
        init_part(part, pyramid_layout.size_being_placed, i);
        pyramid_part_mark_terrain(part_id);
        prev_id = part_id;
    }
    int32_t main_id = monument_main_part_id(prev_id);
    pyramid_init_preset_parts(main_id);
    pyramid_create_entry_tile(main_id);
}

void pyramid_create_entry_tile(int32_t building_id)
{
    if (building_get(building_id)->type != BUILDING_PYRAMID) {
        return;
    }
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    int32_t offset = main->grid_offset + (scenario_pyramid_style == PYRAMID_STYLE_MASTABA ? mastaba_entry_tile_offset[size] : pyramid_entry_tile_offset[size]);
    int32_t x = offset % GRID_SIZE - map_info.diamond_x_min;
    int32_t y = offset / GRID_SIZE - map_info.diamond_y_min;
    int32_t param = building_property_get(BUILDING_PYRAMID_ENTRY_TILE)->size;
    int32_t tile_id = building_create(city_current_player, BUILDING_PYRAMID_ENTRY_TILE, x, y, 0, 0);
    game_undo_add_building(tile_id);
    building_construction_add_to_map(city_current_player, BUILDING_PYRAMID_ENTRY_TILE, tile_id, x, y, param, 0, 0);
    building_get(tile_id)->subtype = (int16_t) main_id;
}

void pyramid_remove_entry_tile(int32_t building_id, bool delete_it)
{
    int32_t main_id = monument_main_part_id(building_id);
    building *main = building_get(main_id);
    int32_t size = main->data.pyramid.size_variant;
    int32_t offset = main->grid_offset + (scenario_pyramid_style == PYRAMID_STYLE_MASTABA ? mastaba_entry_tile_offset[size] : pyramid_entry_tile_offset[size]);
    int32_t tile_id = map_grids.building_ids[offset];
    if (tile_id == 0) {
        return;
    }
    building *tile = building_get(tile_id);
    if (tile->type != BUILDING_PYRAMID_ENTRY_TILE) {
        return;
    }
    if (delete_it) {
        game_undo_add_building(tile_id);
        tile->state = BUILDING_STATE_DELETED;
        routed_building_items_placed++;
    } else {
        tile->state = BUILDING_STATE_UNDO;
    }
}

void pyramid_get_entry_tile(int32_t building_id, int32_t *grid_offset, int32_t *x, int32_t *y)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t size = building_get(main_id)->data.pyramid.size_variant;
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        const building *part = building_get(main_id + mastaba_entry.entry_points[size][0].part);
        *x = part->x;
        int32_t tile_y = part->y + 2;
        *y = tile_y;
        *grid_offset = map_info.grid_start + tile_y * GRID_SIZE + *x;
        return;
    }
    const building *part = building_get(main_id + pyramid_ramp_table[size][0].part);
    int32_t kind = pyramid_layout.pyramid[part->data.pyramid.size_variant][part->subtype].kind;
    *x = part->x;
    *y = part->y;
    switch (kind) {
        case PYRAMID_PART_EDGE_TOP_LEFT:
            (*x)--;
            break;
        case PYRAMID_PART_EDGE_BOTTOM_LEFT:
            *y = part->y + 2;
            break;
        case PYRAMID_PART_EDGE_BOTTOM_RIGHT:
            *x += 2;
            break;
        case PYRAMID_PART_EDGE_TOP_RIGHT:
            *y = part->y - 1;
            break;
    }
    *grid_offset = map_info.grid_start + *y * GRID_SIZE + *x;
}

void pyramid_get_entry_road_access(int32_t building_id, int32_t *x, int32_t *y)
{
    int32_t main_id = monument_main_part_id(building_id);
    const building *part = building_get(main_id + entry_part_offset(building_get(main_id)->data.pyramid.size_variant));
    *x = part->road_access_x;
    *y = part->road_access_y;
}

int32_t pyramid_set_figure_destination_entry(int32_t figure_id, int32_t building_id)
{
    int32_t main_id = monument_main_part_id(building_id);
    int32_t part_id = main_id + entry_part_offset(building_get(main_id)->data.pyramid.size_variant);
    const building *part = building_get(part_id);
    figure *f = figure_get(figure_id);
    f->destination_x = part->road_access_x;
    f->destination_y = part->road_access_y;
    f->destination_grid_offset = part->grid_offset;
    f->roam_length = 0;
    return part_id;
}

int32_t pyramid_has_complex_temple(int32_t size)
{
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        return 0;
    }
    return pyramid_complex_temple_offset[size] != 0;
}

int32_t pyramid_find_by_complex_building(int32_t building_id)
{
    pyramid_complex_search_building = building_id;
    for (int32_t i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == 0 || b->type != BUILDING_PYRAMID) {
            continue;
        }
        int32_t size = b->data.pyramid.size_variant;
        if (pyramid_has_complex_temple(size)) {
            int32_t offset = pyramid_complex_temple_offset[size] + b->grid_offset - map_info.grid_start;
            pyramid_complex_area_op(offset % GRID_SIZE, offset / GRID_SIZE, PYRAMID_COMPLEX_AREA_FIND_BUILDING, 0);
            if (pyramid_complex_found) {
                return i;
            }
        }
        i += pyramid_part_count_of_size(b->data.pyramid.size_variant) - 1;
    }
    return building_id;
}

void pyramid_clear_complex_area(int32_t building_id)
{
    if (building_get(building_id)->type != BUILDING_PYRAMID) {
        return;
    }
    building *main = building_get(monument_main_part_id(building_id));
    int32_t size = main->data.pyramid.size_variant;
    if (!pyramid_has_complex_temple(size)) {
        return;
    }
    int32_t offset = pyramid_complex_temple_offset[size] + main->grid_offset - map_info.grid_start;
    int32_t x = offset % GRID_SIZE;
    int32_t y = offset / GRID_SIZE;
    pyramid_complex_area_op(x, y, PYRAMID_COMPLEX_AREA_CLEAR, 0);
    int32_t end = pyramid_complex_area_end - map_info.grid_start;
    int32_t x_end = end % GRID_SIZE + 2;
    int32_t y_end = end / GRID_SIZE + 2;
    map_tiles_update_region_empty_land(x, y, x_end, y_end, 0, 0);
    map_tiles_update_empty_land_around(x, y, x_end - x);
    map_tiles_update_region_meadow(x, y, x_end, y_end);
    map_tiles_update_region_floodplain(x, y, x_end, y_end);
    map_tiles_update_region_water(x, y, x_end, y_end, 0);
    map_tiles_update_region_marshland(x, y, x_end, y_end);
    map_tiles_update_region_rubble(x, y, x_end, y_end);
}

int32_t monument_defs_count(void)
{
    return MONUMENT_DEFINITIONS;
}
