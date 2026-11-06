// Monument placement checks and part images: the pyramid site (parts, complex temple area, ramps, each failure a
// reason bit), the royal tombs' site, and the part images of the royal tombs (by chamber kind, stage and view),
// Abu Simbel and the sun temple.

#include "building/monument_place.h"
#include "building/abu_simbel.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/monument.h"
#include "building/monument_object.h"
#include "building/monument_spots.h"
#include "building/pyramid_images.h"
#include "building/royal_tomb.h"
#include "city/kingdom.h"
#include "graphics/image_collection.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/map_data.h"
#include "map/terrain.h"
#include "map/terrain_image_group.h"
#include "widget/widget_data.h"
#include <stdbool.h>
#include <stdint.h>

bool pyramid_can_place(int32_t offset, int32_t size, int32_t *reason)
{
    int32_t tiles = tile_count_for_size[MONUMENT_GHOST_BLOCK_SIZE];
    int32_t view = map_info.orientation / 2;
    *reason = 0;
    uint8_t ok = 1;
    for (int32_t part = 0;; part++) {
        int32_t count = (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) ? mastaba_part_count[size] : pyramid_part_count[size];
        if (part >= count) {
            break;
        }
        int32_t base;
        if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
            base = mastaba_part_xy[part].x + mastaba_part_xy[part].y * GRID_SIZE;
        } else {
            base = pyramid_part_grid_offsets[part];
        }
        base += offset;
        for (int32_t i = 0; i < tiles; i++) {
            if (map_grid_is_inside_size(base % GRID_SIZE, base / GRID_SIZE, 3)) {
                return 0;
            }
            if (map_grids.terrain[tile_grid_offsets_base.grid_offsets[view][i] + base] & TERRAIN_BLOCKS_MONUMENT_SITE) {
                ok = 0;
                *reason |= PYRAMID_PLACE_PART_TILES_BLOCKED;
            }
        }
    }
    if (pyramid_has_complex_temple(size)) {
        int32_t at = pyramid_complex_temple_offset[size] + offset - map_info.grid_start;
        pyramid_complex_area_op(at % GRID_SIZE, at / GRID_SIZE, PYRAMID_COMPLEX_AREA_WALK, 0);
        if (pyramid_complex_area_end == 0) {
            ok = 0;
            *reason |= PYRAMID_PLACE_COMPLEX_TEMPLE_BLOCKED;
        }
    }
    int32_t count, start;
    if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
        count = mastaba_ramp_count[size];
        start = mastaba_ramp_offset[size];
    } else {
        count = pyramid_ramp_tile_count[size];
        start = pyramid_ramp_offset[size];
    }
    if (count > 0) {
        const uint32_t *tile = &map_grids.terrain[start + offset];
        for (; count != 0; count--, tile++) {
            if (*tile & TERRAIN_BLOCKS_MONUMENT_RAMP) {
                ok = 0;
                *reason |= PYRAMID_PLACE_RAMP_BLOCKED;
            }
        }
    }
    return ok;
}

bool royal_tomb_can_place(tomb_object *self, int32_t offset, const monument_part_desc *descs, int32_t count)
{
    int32_t x0 = (offset - map_info.grid_start) % GRID_SIZE;
    int32_t y0 = (offset - map_info.grid_start) / GRID_SIZE;
    uint8_t ok = 1;
    if (count <= 0) {
        return ok;
    }
    for (int32_t part = 0; part < count; part++) {
        if (!ok) {
            return ok;
        }
        const monument_part_desc *d = &descs[part];
        if (part == self->last_part) {
            int32_t at = map_info.grid_start + (d->dy + y0) * GRID_SIZE + d->dx + x0;
            uint8_t any = 0;
            if (self->vtable->tile_is_free(self, map_grids.terrain[at - GRID_SIZE], part)) {
                any = 1;
            }
            if (self->vtable->tile_is_free(self, map_grids.terrain[at + GRID_SIZE], part)) {
                any = 1;
            }
            if (self->vtable->tile_is_free(self, map_grids.terrain[at - 1], part)) {
                any = 1;
            }
            if (!self->vtable->tile_is_free(self, map_grids.terrain[at + 1], part) && !any) {
                ok = 0;
            }
        }
        int32_t by = d->dy + y0;
        int32_t bx = d->dx + x0;
        map_tile_cursor_offset = map_info.grid_start + by * GRID_SIZE + bx;
        int32_t size = d->size;
        for (int32_t r = 0; r < size && ok; r++) {
            for (int32_t c = 0; c < d->size && ok; c++) {
                if (map_grid_is_inside_size(map_info.diamond_x_min + c + bx, map_info.diamond_y_min + r + by, 3)) {
                    ok = 0;
                } else {
                    ok = self->vtable->tile_is_free(self, map_grids.terrain[building_square_tile_offset(r, c) + map_tile_cursor_offset], part);
                }
            }
        }
    }
    return ok;
}

bool small_royal_tomb_can_place(tomb_object *self, int32_t offset)
{
    return royal_tomb_can_place(self, offset, royal_tomb_record_of(self)->small.descs, SMALL_TOMB_PARTS);
}
bool medium_royal_tomb_can_place(tomb_object *self, int32_t offset)
{
    return royal_tomb_can_place(self, offset, royal_tomb_record_of(self)->medium.descs, MEDIUM_TOMB_PARTS);
}
bool large_royal_tomb_can_place(tomb_object *self, int32_t offset)
{
    return royal_tomb_can_place(self, offset, royal_tomb_record_of(self)->large.descs, LARGE_TOMB_PARTS);
}
bool grand_royal_tomb_can_place(tomb_object *self, int32_t offset)
{
    return royal_tomb_can_place(self, offset, royal_tomb_record_of(self)->grand.descs, GRAND_TOMB_PARTS);
}

static int32_t tomb_part_image(enum royal_tomb_size size, int32_t chambers, tomb_object *self, int32_t id, int32_t *flags)
{
    const royal_tomb_image_set *t = &royal_tomb_image_sets[size];
    building *b = building_get(id);
    *flags = 0;
    int32_t stage = b->data.royal_tomb.stage;
    int32_t kind = b->data.royal_tomb.chamber_kind;
    int32_t entrance = (kind == TOMB_CHAMBER_ENTRANCE_A) || (kind == TOMB_CHAMBER_ENTRANCE_B) || (kind == TOMB_CHAMBER_ENTRANCE_C);
    if (stage == TOMB_STAGE_CLOSED && !entrance) {
        *flags = MONUMENT_PART_IMAGE_PER_TILE;
        return b->data.royal_tomb.closed_image_variant + image_group(IMAGE_GROUP_ROYAL_TOMB_CLOSED_PART);
    }
    int32_t rot = b->data.royal_tomb.anim_key - map_info.orientation / 2;
    if (rot < 0) {
        rot += 4;
    }
    int32_t add = 0;
    if ((stage >= TOMB_STAGE_FIRST_BUILT_IMAGE) && (stage <= TOMB_STAGE_LAST_BUILT_IMAGE)) {
        add = (stage - TOMB_STAGE_FIRST_BUILT_IMAGE + 1) * TOMB_VIEW_IMAGES;
    }
    if (t->last_part_closed && b->subtype == self->last_part) {
        add = 0;
    }
    uint8_t finished = b->data.royal_tomb.completed_image;
    if ((city_victory_goals_reached() && finished && royal_tomb_get_progress(self, id, chambers, 0) == 100) ||
        ((int16_t) b->created_sequence == MONUMENT_KEPT_FROM_EARLIER_MISSION && b->data.royal_tomb.completed_image)) {
        add += TOMB_VIEW_IMAGES;
    }
    kind = b->data.royal_tomb.chamber_kind;
    if (kind == TOMB_CHAMBER_SEALING) {
        return image_group(t->sealing_group) + (t->sealing_adds_stage ? add : 0);
    }
    if (kind == TOMB_CHAMBER_ENTRANCE_A) {
        return image_group(t->entrance_groups[ROYAL_TOMB_ENTRANCE_A]) + rot;
    }
    if (kind == TOMB_CHAMBER_ENTRANCE_B) {
        return image_group(t->entrance_groups[ROYAL_TOMB_ENTRANCE_B]) + rot;
    }
    if (kind == TOMB_CHAMBER_ENTRANCE_C) {
        return image_group(t->entrance_groups[ROYAL_TOMB_ENTRANCE_C]) + rot;
    }
    for (int32_t i = 0; i < t->chamber_count; i++) {
        if (t->chambers[i].kind == kind) {
            int32_t group = t->chambers[i].group;
            return self->idle ? image_group(group + TOMB_IDLE_GROUP_DISTANCE) + rot : image_group(group) + add + rot;
        }
    }
    return 0;
}

int32_t small_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags)
{
    return tomb_part_image(ROYAL_TOMB_SMALL, SMALL_TOMB_CHAMBERS, self, building_id, flags);
}
int32_t medium_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags)
{
    return tomb_part_image(ROYAL_TOMB_MEDIUM, MEDIUM_TOMB_CHAMBERS, self, building_id, flags);
}
int32_t large_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags)
{
    return tomb_part_image(ROYAL_TOMB_LARGE, LARGE_TOMB_CHAMBERS, self, building_id, flags);
}
int32_t grand_royal_tomb_part_image(tomb_object *self, int32_t building_id, int32_t *flags)
{
    return tomb_part_image(ROYAL_TOMB_GRAND, GRAND_TOMB_CHAMBERS, self, building_id, flags);
}

int32_t abu_simbel_part_image(monument_object *object, int32_t id, int32_t *flags)
{
    abu_simbel_object_t *self = (abu_simbel_object_t *) object;
    static const int32_t facades[ANIM_VIEWS] = { IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_0, IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_1,
        IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_2, IMAGE_GROUP_ABU_SIMBEL_FACADE_VIEW_3 };
    static const int32_t statues[ANIM_VIEWS] = { IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_0, IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_1,
        IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_2, IMAGE_GROUP_ABU_SIMBEL_STATUE_VIEW_3 };
    const building *b = building_get(id);
    int32_t kind = self->parts[b->subtype].kind;
    int32_t view = map_info.orientation / 2;
    int32_t front = view == 0 || view == 3;
    *flags = 0;
    anim_load_script("AbuSimbel", MONUMENT_SCRIPT_SLOT_ABU_SIMBEL);
    int32_t stage = b->data.monument.stage;
    uint32_t odd = (uint32_t) view & 1;
    if ((kind >= 0 && kind <= 6) && (view < 0 || view > 3)) {
        return 0;
    }
    switch (kind) {
        case 0:
            return image_group(facades[view]);
        case 1:
            return image_group(facades[(view + 1) & 3]);
        case 2:
            return image_group(facades[(view + 3) & 3]);
        case 3:
            return image_group(statues[view]);
        case 4:
            return image_group(statues[(view + 3) & 3]);
        case 5:
            return image_group(statues[(view + 1) & 3]);
        case 6:
            return image_group(statues[(view + 2) & 3]);
        case 7:
            if (front) {
                return image_group(IMAGE_GROUP_ABU_SIMBEL_PART7_FRONT) + odd + stage * 2;
            }
            return ((-1 - view) & 1) + image_group(IMAGE_GROUP_ABU_SIMBEL_PART7_SIDE);
        case 8:
            if (front) {
                return image_group(IMAGE_GROUP_ABU_SIMBEL_PART8) + odd + stage * 2;
            }
            return odd + 14 + image_group(IMAGE_GROUP_ABU_SIMBEL_PART8);
        case 9:
            if (stage > 0) {
                return image_group(IMAGE_GROUP_ABU_SIMBEL_PART9) + odd + stage * 2;
            }
            return odd + image_group(IMAGE_GROUP_ABU_SIMBEL_PART9);
        case 10:
        case 11:
            if (front) {
                return image_group(IMAGE_GROUP_ABU_SIMBEL_PART10_FRONT) + odd + stage * 2;
            }
            return ((-1 - view) & 1) + image_group(IMAGE_GROUP_ABU_SIMBEL_PART10_SIDE);
        default:
            return 0;
    }
}

int32_t sun_temple_part_image(int32_t id, int32_t *flags)
{
    *flags = 0;
    const building *b = building_get(id);
    int32_t stage = b->data.monument.stage;
    int32_t view = map_info.orientation;
    switch (sun_temple_part_descs[b->subtype].kind) {
        case 0: {
            int32_t script;
            if (stage < 9) {
                script = 0;
            } else {
                script = (stage >= 13) + 1;
            }
            int32_t which = b->data.monument.city_slot;
            if (which == 1) {
                anim_load_script(sun_temple_scripts[script], MONUMENT_SCRIPT_SLOT_STAGE_B);
                return ((view / 2) & 1) + image_group(IMAGE_GROUP_SPHINX_MIDDLE);
            }
            if (which == 2) {
                anim_load_script(sun_temple_scripts[script], MONUMENT_SCRIPT_SLOT_STAGE_C);
                return ((view / 2) & 1) + image_group(IMAGE_GROUP_SPHINX_END_B);
            }
            anim_load_script(sun_temple_scripts[script], MONUMENT_SCRIPT_SLOT_STAGE_A);
            return ((view / 2) & 1) + image_group(IMAGE_GROUP_SPHINX_END_A);
        }
        case 1:
            if (stage == 2) {
                return ((view / 2) & 1) + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            break;
        case 2:
            if (stage == 2) {
                return (((8 - view) / 2) & 3) + 2 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            break;
        case 3:
            if (stage == 2) {
                return (((8 - view) / 2 + 1) & 3) + 2 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            break;
        case 4:
            if (stage == 2) {
                return (((8 - view) / 2 - 2) & 3) + 2 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            break;
        case 5:
            if (stage == 2) {
                return (((8 - view) / 2 - 1) & 3) + 2 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            break;
        case 6:
            if (stage == 2) {
                return ((view / 2) & 1) + 6 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 7:
            if (stage == 2) {
                return ((view / 2 - 1) & 1) + 6 + image_group(IMAGE_GROUP_MAUSOLEUM_GROUND);
            }
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 8:
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            if (b->data.monument.stage == 2) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + 8;
            }
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 9:
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            if (b->data.monument.stage == 2) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + 9;
            }
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 10:
            if (stage == 2) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + 10;
            }
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 11:
            if (stage == 2) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + 11;
            }
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            return (b->data.monument.stage == 1) ? image_group(IMAGE_GROUP_LEVELED_GROUND) : 0;
        case 12:
            if (stage == 4) {
                return image_group(IMAGE_GROUP_MAUSOLEUM_GROUND) + 12;
            }
            *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
            if (b->data.monument.stage < 1) {
                *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_EDGE_TILES | MONUMENT_PART_IMAGE_UNBUILT;
                return image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
            }
            return image_group(IMAGE_GROUP_LEVELED_GROUND);
        default:
            return 0;
    }
    *flags = MONUMENT_PART_IMAGE_PER_TILE | MONUMENT_PART_IMAGE_UNBUILT;
    if (b->data.monument.stage != 1) {
        return image_group(IMAGE_GROUP_MAUSOLEUM_WALKWAY);
    }
    return image_group(IMAGE_GROUP_LEVELED_GROUND);
}
