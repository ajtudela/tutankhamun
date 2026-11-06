// The sun temple: a hundred chained part records from its descriptor table. The main temple is built by
// carpenters and stonemasons through its work spot table, the other parts by laborers who also fetch sandstone;
// with the progress report, the part images and the stone blocks drawn by stage.
//
// The laborers and the part descriptors have these quirks:
// - a laborer is killed when no storage yard has sandstone;
// - one part kind gives the laborer another terrain usage;
// - a part descriptor copy takes more bytes than are set.

#include "building/sun_temple.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_place.h"
#include "building/monument_spots.h"
#include "building/monument_static.h"
#include "city/resource_type.h"
#include "core/static_init.h"
#include "figure/action.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"

static building *main_of(int32_t building_id)
{
    return building_get(monument_main_part_id(building_id));
}

static int32_t part_kind(int32_t building_id)
{
    return sun_temple_part_descs[building_get(building_id)->subtype].kind;
}

static monument_work_spot *spot_entry(int32_t index)
{
    return &sun_temple_work_spots[index];
}

void sun_temple_animate_spots(void)
{
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        monument_spot_animate(spot_entry(i)->spot);
    }
}

void sun_temple_reset_work_spots(void)
{
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        monument_work_spot *entry = spot_entry(i);
        entry->spot->figure_id = 0;
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_IDLE);
        entry->spot->work_remaining = entry->initial_work;
    }
}

void sun_temple_draw_spots(int32_t x, int32_t y)
{
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        int32_t frame_x, frame_y, image;
        monument_spot_get_draw_info(spot_entry(i)->spot, &frame_x, &frame_y, &image);
        if (image != 0) {
            monument_draw_image(image, frame_x + x, frame_y + y);
        }
    }
}

void sun_temple_check_stage_done(int32_t building_id)
{
    int32_t stage = building_get(building_id)->data.monument.stage;
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        if (spot_entry(i)->stage == stage && spot_entry(i)->spot->work_remaining > 0) {
            return;
        }
    }
    sun_temple_advance_stage(building_id);
}

static monument_work_spot *find_free_spot(int32_t stage, int32_t figure_type)
{
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        monument_work_spot *entry = spot_entry(i);
        if (entry->spot->figure_id == 0 && entry->stage == stage && entry->figure_type == figure_type && entry->spot->work_remaining > 0) {
            return entry;
        }
    }
    return 0;
}

void sun_temple_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    figure *f = figure_get(figure_id);
    int32_t stage = building_get(building_id)->data.monument.stage;
    f->is_ghost = 1;
    monument_work_spot *entry = 0;
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        if (spot_entry(i)->spot->figure_id == figure_id) {
            entry = spot_entry(i);
            if (entry->stage != stage) {
                f->action_state = next_action;
                return;
            }
            break;
        }
    }
    if (!entry) {
        entry = find_free_spot(stage, f->type);
        if (!entry) {
            f->action_state = next_action;
            return;
        }
    }
    if (entry->spot->figure_id == 0) {
        monument_spot_attach_figure(entry->spot, figure_id);
        monument_spot_start_anim(entry->spot, MONUMENT_SPOT_LOOP);
    }
    entry->spot->work_remaining--;
    if (entry->spot->work_remaining < 1) {
        sun_temple_check_stage_done(building_id);
        f->action_state = next_action;
    }
}

int32_t sun_temple_has_free_spot(int32_t building_id, int32_t figure_id)
{
    int32_t stage = building_get(building_id)->data.monument.stage;
    return find_free_spot(stage, figure_get(figure_id)->type) ? building_id : 0;
}

void sun_temple_count_assigned(int32_t figure_type, int32_t *count)
{
    *count = 0;
    for (int32_t i = 0; i < SUN_TEMPLE_WORK_SPOTS; i++) {
        if (spot_entry(i)->spot->figure_id != 0 && spot_entry(i)->figure_type == figure_type) {
            ++*count;
        }
    }
}

int32_t sun_temple_has_free_spot_for_type(int32_t figure_type, int32_t stage)
{
    return find_free_spot(stage, figure_type) != 0;
}

void sun_temple_place(int32_t x, int32_t y, int32_t building_id)
{
    int32_t slot = monument_register(building_id);
    const monument_part_desc *desc = &sun_temple_part_descs[0];
    building *b = building_get(building_id);
    b->x = (int16_t) (x + desc->dx);
    b->y = (int16_t) (y + desc->dy);
    b->grid_offset = map_info.grid_start + (y + desc->dy) * GRID_SIZE + desc->dx + x;
    b->subtype = 0;
    b->size = (uint8_t) desc->size;
    b->prev_part_building_id = 0;
    b->data.monument.city_slot = slot;
    b->data.monument.stage = 0;
    b->data.monument.is_main_part = 1;
    b->data.monument.worker_figure_id = 0;
    sun_temple_update_image(building_id);
    game_undo_add_building(building_id);
    int32_t prev_id = building_id;
    for (int32_t part = 1; part < SUN_TEMPLE_PARTS; part++) {
        desc = &sun_temple_part_descs[part];
        int32_t id = building_create(city_current_player, BUILDING_SUN_TEMPLE, desc->dx + x, desc->dy + y, 0, prev_id + 1);
        building *p = building_get(id);
        p->prev_part_building_id = (int16_t) prev_id;
        building_get(prev_id)->next_part_building_id = (int16_t) id;
        p->next_part_building_id = 0;
        p->subtype = (int16_t) part;
        p->size = (uint8_t) desc->size;
        p->data.monument.city_slot = slot;
        p->data.monument.stage = 0;
        p->data.monument.is_main_part = 0;
        p->data.monument.worker_figure_id = 0;
        sun_temple_update_image(id);
        game_undo_add_building(id);
        prev_id = id;
    }
}

void sun_temple_update_image(int32_t building_id)
{
    int32_t flags;
    int32_t image = sun_temple_part_image(building_id, &flags);
    building *b = building_get(building_id);
    int32_t size = sun_temple_part_descs[b->subtype].size;
    if ((flags & MONUMENT_PART_IMAGE_PER_TILE) == 0) {
        map_building_tiles_add(building_id, b->x, b->y, size, size, image, b->subtype == 0 ? TERRAIN_BUILDING : TERRAIN_MONUMENT | TERRAIN_BUILDING);
        return;
    }
    map_tile_cursor_offset = map_info.grid_start + b->x + b->y * GRID_SIZE;
    for (int32_t row = 0; row < size; row++) {
        for (int32_t col = 0; col < size; col++) {
            int32_t offset = building_square_tile_offset(row, col) + map_tile_cursor_offset;
            map_grids.terrain[offset] |= TERRAIN_MONUMENT;
            map_grids.building_ids[offset] = (int16_t) building_id;
            map_grids.bitfields[offset] &= (uint8_t) ~(MAP_BITFIELD_SIZE | MAP_BITFIELD_OVERLAY);
            map_grids.edge[offset] |= MAP_EDGE_DRAW_TILE;
            if ((flags & MONUMENT_PART_IMAGE_EDGE_TILES) && ((col != 0 && col != size - 1) || (row != 0 && row != size - 1))) {
                continue;
            }
            if (image != 0) {
                if (image == image_group(IMAGE_GROUP_LEVELED_GROUND)) {
                    map_grids.images[offset] = (map_grids.random[offset] % ALTERNATE_GRASS_IMAGES) + image;
                } else {
                    map_grids.images[offset] = image;
                }
            }
        }
    }
}

static int32_t live_action(const figure *f)
{
    int32_t action = f->action_state;
    if (action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY) {
        action = f->action_state_before_attack;
    }
    return action;
}

int32_t sun_temple_part_has_active_worker(int32_t building_id)
{
    building *b = building_get(building_id);
    int32_t worker = b->data.monument.worker_figure_id;
    if (worker == 0) {
        return 0;
    }
    figure *f = figure_get(worker);
    if (f->state == FIGURE_STATE_NONE) {
        b->data.monument.worker_figure_id = 0;
        return 0;
    }
    int32_t action = live_action(f);
    switch (f->type) {
        case FIGURE_CARPENTER:
            if (action == FIGURE_ACTION_CARPENTER_GO_TO_SITE || action == FIGURE_ACTION_CARPENTER_WORK) {
                return 1;
            }
            break;
        case FIGURE_STONEMASON:
            if (action == FIGURE_ACTION_STONEMASON_GO_TO_SITE || action == FIGURE_ACTION_STONEMASON_WORK || action == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                return 1;
            }
            break;
        case FIGURE_LABORER:
            if (action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD || action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD ||
                action == FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE || action == FIGURE_ACTION_LABORER_TO_SUN_TEMPLE ||
                action == FIGURE_ACTION_LABORER_SUN_TEMPLE_WORK) {
                return 1;
            }
            break;
    }
    b->data.monument.worker_figure_id = 0;
    return 0;
}

static void send_to_part(figure *f, const building *b)
{
    f->destination_x = b->road_access_x;
    f->destination_y = b->road_access_y;
    f->destination_grid_offset = b->grid_offset;
    f->roam_length = 0;
}

static int32_t assign_laborer(int32_t building_id, int32_t figure_id, bool assign)
{
    building *b = building_get(building_id);
    figure *f = figure_get(figure_id);
    if (b->data.monument.stage != 0 || f->type != FIGURE_LABORER || sun_temple_part_has_active_worker(building_id)) {
        return 0;
    }
    if (assign) {
        f->action_state = FIGURE_ACTION_LABORER_TO_SUN_TEMPLE;
        b->data.monument.worker_figure_id = figure_id;
        send_to_part(f, b);
    }
    return 1;
}

static int32_t assign_material(int32_t building_id, int32_t figure_id, bool assign, int32_t terrain_usage)
{
    building *b = building_get(building_id);
    figure *f = figure_get(figure_id);
    if (f->type != FIGURE_LABORER) {
        if (sun_temple_part_has_active_worker(building_id) || f->type != FIGURE_STONEMASON) {
            return 0;
        }
        if (assign) {
            f->action_state = FIGURE_ACTION_STONEMASON_GO_TO_SITE;
            b->data.monument.worker_figure_id = figure_id;
            send_to_part(f, b);
        }
        return 1;
    }
    if (!sun_temple_part_has_active_worker(building_id)) {
        return 0;
    }
    int32_t mason_id = b->data.monument.worker_figure_id;
    figure *mason = figure_get(mason_id);
    int32_t partner_id = mason->leading_figure_id;
    if (partner_id != 0) {
        figure *partner = figure_get(partner_id);
        if (partner->state != FIGURE_STATE_NONE && partner->type == FIGURE_LABORER) {
            int32_t action = partner->action_state;
            if ((action == FIGURE_ACTION_ON_FERRY || action == FIGURE_ACTION_WAIT_FOR_FERRY || action == FIGURE_ACTION_LABORER_TO_STORAGE_YARD ||
                    action == FIGURE_ACTION_LABORER_AT_STORAGE_YARD || action == FIGURE_ACTION_LABORER_SLED_TO_SUN_TEMPLE) &&
                partner->leading_figure_id == mason_id) {
                return 0;
            }
        }
        mason->leading_figure_id = 0;
    }
    if (!assign) {
        return 1;
    }
    int32_t storage_id = building_storageyard_for_getting(building_id, RESOURCE_SANDSTONE, 1);
    if (storage_id == 0) {
        f->state = FIGURE_STATE_DEAD;
        return 1;
    }
    f->collecting_item_id = RESOURCE_SANDSTONE;
    f->resource_amount = 0;
    f->destination_building_id = (int16_t) storage_id;
    f->leading_figure_id = (int16_t) mason_id;
    f->action_state = FIGURE_ACTION_LABORER_TO_STORAGE_YARD;
    f->wait_ticks = 0;
    f->destination_x = (int16_t) map_result_x;
    f->destination_y = (int16_t) map_result_y;
    if (terrain_usage) {
        f->terrain_usage = (uint8_t) terrain_usage;
    }
    mason->leading_figure_id = (int16_t) figure_id;
    return 1;
}

int32_t sun_temple_check_work_for_figure(int32_t building_id, int32_t figure_id, bool assign)
{
    building *b = building_get(building_id);
    figure *f = figure_get(figure_id);
    uint32_t flags = (uint32_t) main_of(building_id)->data.monument.work_flags;
    int32_t is_laborer = f->type == FIGURE_LABORER;
    int32_t stage = b->data.monument.stage;
    int32_t kind = part_kind(building_id);
    switch (kind) {
        case 0:
            if (sun_temple_part_has_active_worker(building_id) || !sun_temple_has_free_spot(building_id, figure_id)) {
                return 0;
            }
            if (assign) {
                if (f->type == FIGURE_CARPENTER) {
                    f->action_state = FIGURE_ACTION_CARPENTER_WORK;
                } else if (f->type == FIGURE_STONEMASON) {
                    f->action_state = FIGURE_ACTION_STONEMASON_WORK;
                }
                b->data.monument.worker_figure_id = figure_id;
                send_to_part(f, b);
                f->wait_ticks = 0;
            }
            return 1;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7: {
            uint32_t unlock = (kind == 1) ? SUN_TEMPLE_TEMPLE_DONE : SUN_TEMPLE_KIND1_DONE;
            if (!(flags & unlock) || stage == 0) {
                return is_laborer ? assign_laborer(building_id, figure_id, assign) : 0;
            }
            if (stage != SUN_TEMPLE_PART_STARTED) {
                return 0;
            }
            return assign_material(building_id, figure_id, assign, TERRAIN_USAGE_ANY);
        }
        case 8:
        case 9:
        case 10:
        case 11:
            if (!is_laborer || sun_temple_part_has_active_worker(building_id)) {
                return 0;
            }
            if (!(flags & SUN_TEMPLE_KIND8_STARTED)) {
                if (stage != 0) {
                    return 0;
                }
            } else if (!(flags & SUN_TEMPLE_TEMPLE_DONE) || stage != 1) {
                return 0;
            }
            if (assign) {
                f->action_state = FIGURE_ACTION_LABORER_TO_SUN_TEMPLE;
                b->data.monument.worker_figure_id = figure_id;
                send_to_part(f, b);
            }
            return 1;
        case 12:
            if (!(flags & SUN_TEMPLE_KIND8_DONE) || !(flags & SUN_TEMPLE_KIND2_DONE) || stage == 0) {
                return is_laborer ? assign_laborer(building_id, figure_id, assign) : 0;
            }
            if (stage < 1 || stage > 3) {
                return 0;
            }
            return assign_material(building_id, figure_id, assign, TERRAIN_USAGE_PREFER_ROADS);
        default:
            return 0;
    }
}

void sun_temple_figure_work_tick(int32_t building_id, int32_t figure_id, uint8_t next_action)
{
    building *b = building_get(building_id);
    figure *f = figure_get(figure_id);
    int32_t kind = part_kind(building_id);
    if (kind == 0) {
        sun_temple_figure_work_tick_spot(building_id, figure_id, next_action);
        return;
    }
    if ((uint32_t) kind > SUN_TEMPLE_PART_KIND_LAST) {
        return;
    }
    if (kind == 1 && f->wait_ticks < 0) {
        f->wait_ticks = 0;
    }
    f->wait_ticks++;
    if (f->wait_ticks < SUN_TEMPLE_LABORER_WORK_TICKS) {
        return;
    }
    b->data.monument.stage++;
    sun_temple_update_image(building_id);
    int32_t main_id = building_id - b->subtype;
    building *main = building_get(main_id);
    if (kind == 1) {
        if (b->data.monument.stage == SUN_TEMPLE_PART_DONE) {
            main->data.monument.work_flags |= SUN_TEMPLE_KIND1_DONE;
        }
    } else if (kind <= 7) {
        if (b->data.monument.stage == SUN_TEMPLE_PART_DONE) {
            int32_t part = 0;
            for (; part < SUN_TEMPLE_PARTS; part++) {
                int32_t k = sun_temple_part_descs[part].kind;
                if (k >= 2 && k <= 7 && building_get(main_id + part)->data.monument.stage < SUN_TEMPLE_PART_DONE) {
                    break;
                }
            }
            if (part == SUN_TEMPLE_PARTS) {
                main->data.monument.work_flags |= SUN_TEMPLE_KIND2_DONE;
            }
        }
    } else if (kind <= 11) {
        int32_t all_started = 1;
        int32_t all_done = 1;
        for (int32_t part = 0; part < SUN_TEMPLE_PARTS; part++) {
            int32_t k = sun_temple_part_descs[part].kind;
            if (k >= 8 && k <= 11) {
                int32_t stage = building_get(main_id + part)->data.monument.stage;
                if (stage == 0) {
                    all_started = 0;
                }
                if (stage < SUN_TEMPLE_PART_DONE) {
                    all_done = 0;
                }
            }
        }
        if (all_started) {
            main->data.monument.work_flags |= SUN_TEMPLE_KIND8_STARTED;
        }
        if (all_done) {
            main->data.monument.work_flags |= SUN_TEMPLE_KIND8_DONE;
        }
    } else if (b->data.monument.stage == SUN_TEMPLE_LAST_PART_DONE) {
        main->data.monument.work_flags |= SUN_TEMPLE_KIND12_DONE;
    }
    b->data.monument.worker_figure_id = 0;
    f->action_state = next_action;
}

int32_t sun_temple_find_work_for_figure(int32_t figure_id)
{
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_SUN_TEMPLE && sun_temple_check_work_for_figure(id, figure_id, 1)) {
            return id;
        }
    }
    return 0;
}

int32_t sun_temple_get_progress(int32_t building_id, int32_t *out)
{
    int32_t main_id = monument_main_part_id(building_id);
    uint32_t flags = (uint32_t) building_get(main_id)->data.monument.work_flags;
    int32_t status = SUN_TEMPLE_STATUS_SITE, at_stage0 = 0, temple_left = 0, kind1 = 0, kind2 = 0, kind8 = 0, kind12 = 0;
    int32_t masons = 0, masons_waiting = 0, laborers = 0, carpenters = 0, material = 0;
    for (int32_t part = 0; part < SUN_TEMPLE_PARTS; part++) {
        int32_t id = main_id + part;
        building *b = building_get(id);
        if (sun_temple_part_has_active_worker(id)) {
            if (b->data.monument.worker_figure_id == 0) {
                return 0;
            }
            figure *f = figure_get(b->data.monument.worker_figure_id);
            if (f->type == FIGURE_CARPENTER) {
                carpenters++;
            } else if (f->type == FIGURE_STONEMASON) {
                if (f->action_state == FIGURE_ACTION_STONEMASON_WAIT_FOR_BLOCK) {
                    masons_waiting++;
                }
                masons++;
            } else if (f->type == FIGURE_LABORER) {
                laborers++;
            }
        }
        int32_t stage = b->data.monument.stage;
        switch (sun_temple_part_descs[b->subtype].kind) {
            case 0:
                temple_left += SUN_TEMPLE_STAGE_COMPLETE - stage;
                if (stage == 0) {
                    status = (flags & SUN_TEMPLE_KIND8_STARTED) ? SUN_TEMPLE_STATUS_TEMPLE_STARTED : SUN_TEMPLE_STATUS_SITE;
                } else if (stage >= 1 && stage <= 3) {
                    status = SUN_TEMPLE_STATUS_TEMPLE_STARTED;
                } else if (stage >= 4 && stage < SUN_TEMPLE_STAGE_COMPLETE) {
                    status = SUN_TEMPLE_STATUS_TEMPLE_RISING;
                } else if (stage == SUN_TEMPLE_STAGE_COMPLETE) {
                    status = SUN_TEMPLE_STATUS_TEMPLE_DONE;
                }
                break;
            case 1:
                if (stage == 0 || stage == 1) {
                    at_stage0 += stage == 0;
                    kind1++;
                    material += MONUMENT_MATERIAL_PER_LOAD;
                }
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                if (stage == 0 || stage == 1) {
                    at_stage0 += stage == 0;
                    kind2++;
                    material += MONUMENT_MATERIAL_PER_LOAD;
                }
                break;
            case 8:
            case 9:
            case 10:
            case 11:
                if (stage == 0 || stage == 1) {
                    at_stage0 += stage == 0;
                    kind8++;
                }
                break;
            case 12:
                if (stage >= 0 && stage < SUN_TEMPLE_LAST_PART_DONE) {
                    at_stage0 += stage == 0;
                    int32_t left = (stage == 0) ? (SUN_TEMPLE_LAST_PART_DONE - 1) : (SUN_TEMPLE_LAST_PART_DONE - stage);
                    kind12 += left;
                    material += left * MONUMENT_MATERIAL_PER_LOAD;
                }
                break;
        }
    }
    int32_t spot_carpenters = 0, spot_masons = 0;
    sun_temple_count_assigned(FIGURE_CARPENTER, &spot_carpenters);
    sun_temple_count_assigned(FIGURE_STONEMASON, &spot_masons);
    int32_t percent = ((SUN_TEMPLE_WORK_UNITS - kind12 - kind8 - kind2 - temple_left - at_stage0) * 100) / SUN_TEMPLE_WORK_UNITS;
    if (out) {
        out[SUN_TEMPLE_PROGRESS_AT_STAGE_0] = at_stage0;
        out[SUN_TEMPLE_PROGRESS_STATUS] = status;
        out[SUN_TEMPLE_PROGRESS_TEMPLE_LEFT] = temple_left;
        out[SUN_TEMPLE_PROGRESS_KIND8_LEFT] = kind8;
        out[SUN_TEMPLE_PROGRESS_KIND1_LEFT] = kind1;
        out[SUN_TEMPLE_PROGRESS_KIND2_LEFT] = kind2;
        out[SUN_TEMPLE_PROGRESS_STONEMASONS] = spot_masons + masons;
        out[SUN_TEMPLE_PROGRESS_STONEMASONS_WAITING] = masons_waiting;
        out[SUN_TEMPLE_PROGRESS_LABORERS] = laborers;
        out[SUN_TEMPLE_PROGRESS_KIND12_LEFT] = kind12;
        out[SUN_TEMPLE_PROGRESS_CARPENTERS] = spot_carpenters + carpenters;
        out[SUN_TEMPLE_PROGRESS_MATERIAL] = material;
        out[SUN_TEMPLE_PROGRESS_PERCENT] = percent;
        out[SUN_TEMPLE_PROGRESS_MATERIAL_TOTAL] = SUN_TEMPLE_MATERIAL_TOTAL;
    }
    return percent;
}

void sun_temple_advance_stage(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->data.monument.stage < SUN_TEMPLE_STAGE_COMPLETE) {
        b->data.monument.stage++;
        if (b->data.monument.stage == SUN_TEMPLE_STAGE_COMPLETE) {
            b->data.monument.work_flags |= SUN_TEMPLE_TEMPLE_DONE;
        }
        sun_temple_update_image(building_id);
    }
}


void sun_temple_draw_stage_blocks(int32_t building_id, int32_t x, int32_t y)
{
    uint32_t stage = (uint32_t) building_get(building_id)->data.monument.stage;
    int32_t count = (stage < SUN_TEMPLE_STAGES) ? sun_temple_stage_blocks[stage] : 0;
    for (int32_t i = 0; i < count; i++) {
        int32_t image = image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + ((i & 1) ? SUN_TEMPLE_BLOCK_IMAGE_B : SUN_TEMPLE_BLOCK_IMAGE_A);
        monument_draw_image(image, x + sun_temple_block_offsets[i].x, sun_temple_block_offsets[i].y + y);
    }
}

int32_t sun_temple_part_size(int32_t building_id)
{
    return sun_temple_part_descs[building_get(building_id)->subtype].size;
}
