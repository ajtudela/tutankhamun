// Building animations of the city drawing's top pass: one helper per animated building type (shipwright with
// its boat, fishing wharf, mines, quarries, venues, dock, water lift, storage yard roof), building_draw_type_anim
// that picks the animation by type, and the food piles on granaries. They only draw.
//
// A granary shows at most as many food piles as it has spots; further piles are not drawn.

#include "widget/city_building_anims.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_type.h"
#include "building/shipwright.h"
#include "city/resources.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_type.h"
#include "graphics/image_blit.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "widget/city_building_sprites.h"
#include "widget/city_view.h"
#include "widget/widget_data.h"
#include <stdint.h>

enum building_anim_image_group {
    IMAGE_GROUP_GRANARY_FOOD_PILES = 283,
    IMAGE_GROUP_ANIM_PAVILION = 583,
    IMAGE_GROUP_ANIM_BOOTH = 584,
    IMAGE_GROUP_ANIM_BANDSTAND_GATE = 586,
    IMAGE_GROUP_ANIM_BANDSTAND_STAGE = 587,
    IMAGE_GROUP_ANIM_SHIPWRIGHT_BOAT = 622,
    IMAGE_GROUP_ANIM_FISHING_WHARF_BOAT = 623,
    IMAGE_GROUP_ANIM_MINE = 625,
    IMAGE_GROUP_ANIM_QUARRY_FIRST = 626,
    IMAGE_GROUP_ANIM_QUARRY_SECOND = 627,
    IMAGE_GROUP_ANIM_SHIPWRIGHT_TRANSPORT_SHIP = 629,
    IMAGE_GROUP_ANIM_SHIPWRIGHT_FISHING_BOAT = 631,
    IMAGE_GROUP_ANIM_FISHING_WHARF = 632,
};

enum building_anim_building_group {
    IMAGE_GROUP_FISHING_WHARF_BUILDING = 515,
    IMAGE_GROUP_SHIPWRIGHT_BUILDING = 523,
    IMAGE_GROUP_BANDSTAND_BUILDING = 361,
};

enum building_anim_more_groups {
    IMAGE_GROUP_STORAGE_YARD_BUILDING = 351,
    IMAGE_GROUP_DOCK_BUILDING = 546,
    IMAGE_GROUP_WATER_LIFT_BUILDING = 547,
    IMAGE_GROUP_ANIM_WATER_LIFT = 578,
    IMAGE_GROUP_ANIM_GRANARY = 624,
    IMAGE_GROUP_ANIM_STORAGE_YARD = 628,
    IMAGE_GROUP_ANIM_DOCK_SHIP = 633,
};

enum building_anim_layout {
    ANIM_ROW_HEIGHT = 30,
    VIEW_IMAGE_MASK = 3,
    DOCK_SHIP_FRAMES_MAX = 20,
    DOCK_SHIP_DOCKED_IMAGES = 4,
    DOCK_FRAMES_PER_STEP = 4,
    WATER_LIFT_X_LESS = 8,
};

enum fishing_wharf_anim {
    FISHING_WHARF_FRAMES_PER_STEP = 4,
    FISHING_WHARF_VIEW_MASK = 3,
    FISHING_WHARF_BOAT_AT_WHARF_IMAGES = 4,
};

static void draw_frame(int32_t group, int32_t anim, int32_t x, int32_t y, const building_anim_offset *offset)
{
    image_draw_sprite_offset(image_group(group) - 1 + anim, x, y, offset->dx, offset->dy);
}

static int32_t shipwright_boat_group(int32_t holder)
{
    int32_t value = holder & BOAT_ORDER_TYPE_MASK;
    int32_t kind = holder & BOAT_ORDER_KIND_MASK;
    if (kind == BOAT_ORDER_FOR_CITY) {
        if (value == FIGURE_FISHING_BOAT) {
            return IMAGE_GROUP_ANIM_SHIPWRIGHT_FISHING_BOAT;
        }
        if (value == FIGURE_TRANSPORT_SHIP) {
            return IMAGE_GROUP_ANIM_SHIPWRIGHT_TRANSPORT_SHIP;
        }
    } else if (kind == BOAT_ORDER_REPAIR) {
        const figure *boat = figure_get(value);
        if (boat->state != FIGURE_STATE_ALIVE || boat->action_state == FIGURE_ACTION_CORPSE) {
            return 0;
        }
        if (boat->type == FIGURE_TRANSPORT_SHIP) {
            return IMAGE_GROUP_ANIM_SHIPWRIGHT_TRANSPORT_SHIP;
        }
    }
    return IMAGE_GROUP_ANIM_SHIPWRIGHT_BOAT;
}

void building_draw_shipwright_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y, int32_t holder)
{
    if (holder == 0) {
        return;
    }
    int32_t group = shipwright_boat_group(holder);
    if (group == 0) {
        return;
    }
    int32_t view = image_id - image_group(IMAGE_GROUP_SHIPWRIGHT_BUILDING);
    draw_frame(group, anim, x, y, &shipwright_anim_offsets[view]);
}

void building_draw_fishing_wharf_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y, int32_t boat)
{
    int32_t index = image_id - image_group(IMAGE_GROUP_FISHING_WHARF_BUILDING);
    int32_t view = (index - 1) & FISHING_WHARF_VIEW_MASK;
    int32_t group = IMAGE_GROUP_ANIM_FISHING_WHARF;
    if (boat != 0 && figure_get(boat)->action_state == FIGURE_ACTION_FISHING_BOAT_AT_WHARF) {
        group = IMAGE_GROUP_ANIM_FISHING_WHARF_BOAT;
        index += FISHING_WHARF_BOAT_AT_WHARF_IMAGES;
    }
    const building_anim_offset *offset = &fishing_wharf_anim_offsets[index];
    image_draw_sprite_offset(image_group(group) + ((anim - 1) * FISHING_WHARF_FRAMES_PER_STEP) + view, x, y, offset->dx, offset->dy);
}

void building_draw_mine_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_MINE, anim, x, y, &building_anim_offsets[BUILDING_ANIM_MINE]);
}

void building_draw_quarry_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_QUARRY_FIRST, anim, x, y, &building_anim_offsets[BUILDING_ANIM_QUARRY_FIRST]);
    draw_frame(IMAGE_GROUP_ANIM_QUARRY_SECOND, anim, x, y, &building_anim_offsets[BUILDING_ANIM_QUARRY_SECOND]);
}

void building_draw_bandstand_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    if (image_id == image_group(IMAGE_GROUP_BANDSTAND_BUILDING)) {
        draw_frame(IMAGE_GROUP_ANIM_BANDSTAND_GATE, anim, x, y, &building_anim_offsets[BUILDING_ANIM_BANDSTAND_GATE]);
    } else {
        draw_frame(IMAGE_GROUP_ANIM_BANDSTAND_STAGE, anim, x, y, &building_anim_offsets[BUILDING_ANIM_BANDSTAND_STAGE]);
    }
}

void building_draw_conservatory_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_BANDSTAND_STAGE, anim, x, y, &building_anim_offsets[BUILDING_ANIM_CONSERVATORY]);
}

void building_draw_booth_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_BOOTH, anim, x, y, &building_anim_offsets[BUILDING_ANIM_BOOTH]);
}

void building_draw_pavilion_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_PAVILION, anim, x, y, &building_anim_offsets[BUILDING_ANIM_PAVILION]);
}

void building_draw_dance_school_anim(int32_t image_id, int32_t anim, int32_t x, int32_t y)
{
    (void) image_id;
    draw_frame(IMAGE_GROUP_ANIM_PAVILION, anim, x, y, &building_anim_offsets[BUILDING_ANIM_DANCE_SCHOOL]);
}

static int32_t granary_pile_count(int32_t stored)
{
    if (stored == 0) {
        return 0;
    }
    for (int32_t piles = 1; piles <= granary_pile_rules[GRANARY_PILE_RULE_MAX_PILES]; piles++) {
        int32_t limit = granary_pile_rules[GRANARY_PILE_RULE_FIRST_AMOUNT] + ((piles - 1) * granary_pile_rules[GRANARY_PILE_RULE_AMOUNT_STEP]);
        int32_t full = granary_pile_rules[GRANARY_PILE_RULE_FULL_AMOUNT];
        if (stored <= (limit > full ? full : limit)) {
            return piles;
        }
    }
    return 0;
}

void draw_granary_stores(int32_t building_id, int32_t grid_offset, int32_t x, int32_t y)
{
    uint8_t edge = map_grids.edge[grid_offset];
    city_resource_determine_available();
    if (!(edge & MAP_EDGE_DRAW_TILE)) {
        return;
    }
    const building *granary = building_get(building_id);
    int32_t spot = 0;
    for (int32_t i = 0; i < city_foods.count; i++) {
        int32_t resource = city_foods.list[i];
        int32_t piles = granary_pile_count(granary->data.granary.resource_stored[resource]);
        for (; piles != 0 && spot < GRANARY_PILE_SPOTS; piles--, spot++) {
            const building_anim_offset *place = &granary_pile_spots[spot];
            image_draw(resource - 1 + image_group(IMAGE_GROUP_GRANARY_FOOD_PILES), place->dx + x, place->dy + y);
        }
    }
}

void building_draw_storage_yard_roof(int32_t image_id, int32_t x, int32_t y)
{
    const building_anim_offset *offset = &building_anim_offsets[BUILDING_ANIM_STORAGE_YARD_ROOF];
    image_draw(image_id, x + offset->dx, y + offset->dy);
}

static void draw_sized_anim(int32_t image_id, int32_t frame, int32_t size, int32_t x, int32_t y)
{
    int32_t dx;
    int32_t dy;
    if (!city_view_footprint_offset(size, &dx, &dy)) {
        return;
    }
    const image_header *header = image_get(image_id);
    int32_t anim_y = header->sprite_offset_y - header->height - dy + (ANIM_ROW_HEIGHT * (size + 1)) + y;
    image_draw(frame + image_id, header->sprite_offset_x + x, anim_y);
}

static void draw_dock_anim(const building *b, int32_t image_id, int32_t frame, int32_t x, int32_t y)
{
    int32_t ship = b->data.dock.ship_id;
    uint32_t index = (image_id - image_collection_group(IMAGE_GROUP_DOCK_BUILDING)) & VIEW_IMAGE_MASK;
    uint32_t view = index - 1;
    int32_t group = IMAGE_GROUP_ANIM_FISHING_WHARF;
    int32_t step = frame;
    if (ship != 0 && figure_get(ship)->action_state == FIGURE_ACTION_TRADE_SHIP_DOCKED) {
        index += DOCK_SHIP_DOCKED_IMAGES;
        group = IMAGE_GROUP_ANIM_DOCK_SHIP;
        if (frame > DOCK_SHIP_FRAMES_MAX) {
            step = DOCK_SHIP_FRAMES_MAX;
        }
    }
    const building_anim_offset *offset = &dock_anim_offsets[index];
    image_draw_sprite_offset(
        image_collection_group(group) + (view & VIEW_IMAGE_MASK) + ((step - 1) * DOCK_FRAMES_PER_STEP), x, y, offset->dx, offset->dy);
}

static void draw_water_lift_anim(int32_t image_id, int32_t frame, int32_t x, int32_t y)
{
    int32_t view = (image_id - image_collection_group(IMAGE_GROUP_WATER_LIFT_BUILDING)) & VIEW_IMAGE_MASK;
    const building_anim_offset *offset = &water_lift_anim_offsets[view];
    int32_t first = image_collection_group(IMAGE_GROUP_ANIM_WATER_LIFT + view);
    image_draw_sprite_offset(first - 1 + frame, x, y, offset->dx - WATER_LIFT_X_LESS + (city_view.tile_half_width * 2), offset->dy + city_view.tile_half_height);
}

void building_draw_type_anim(const building *b, int32_t image_id, int32_t frame, int32_t size, int32_t x, int32_t y)
{
    switch (b->type) {
        case BUILDING_BANDSTAND:
            building_draw_bandstand_anim(image_id, frame, x, y);
            break;
        case BUILDING_BOOTH:
            building_draw_booth_anim(image_id, frame, x, y);
            break;
        case BUILDING_PAVILION:
            building_draw_pavilion_anim(image_id, frame, x, y);
            break;
        case BUILDING_CONSERVATORY:
            building_draw_conservatory_anim(image_id, frame, x, y);
            break;
        case BUILDING_DANCE_SCHOOL:
            building_draw_dance_school_anim(image_id, frame, x, y);
            break;
        case BUILDING_GRANARY:
        case BUILDING_GRANARY_UP:
            draw_frame(IMAGE_GROUP_ANIM_GRANARY, frame, x, y, &building_anim_offsets[BUILDING_ANIM_GRANARY]);
            break;
        case BUILDING_STORAGE_YARD:
        case BUILDING_STORAGE_YARD_UP:
            draw_frame(IMAGE_GROUP_ANIM_STORAGE_YARD, frame, x, y, &building_anim_offsets[BUILDING_ANIM_STORAGE_YARD]);
            building_draw_storage_yard_roof(image_collection_group(IMAGE_GROUP_STORAGE_YARD_BUILDING) + STORAGE_YARD_ROOF_IMAGE, x, y);
            break;
        case BUILDING_SHIPWRIGHT:
            building_draw_shipwright_anim(image_id, frame, x, y, (int16_t) b->data.shipyard.boat_order);
            break;
        case BUILDING_DOCK:
        case BUILDING_DOCK_UP:
            draw_dock_anim(b, image_id, frame, x, y);
            break;
        case BUILDING_FISHING_WHARF:
            building_draw_fishing_wharf_anim(image_id, frame, x, y, b->data.industry.worker_figure_id);
            break;
        case BUILDING_WATER_LIFT:
            draw_water_lift_anim(image_id, frame, x, y);
            break;
        case BUILDING_STONE_QUARRY:
        case BUILDING_LIMESTONE_QUARRY:
        case BUILDING_GRANITE_QUARRY:
        case BUILDING_SANDSTONE_QUARRY:
            building_draw_quarry_anim(image_id, frame, x, y);
            break;
        case BUILDING_GOLD_MINE:
        case BUILDING_GEMSTONE_MINE:
        case BUILDING_COPPER_MINE:
            building_draw_mine_anim(image_id, frame, x, y);
            break;
        default:
            draw_sized_anim(image_id, frame, size, x, y);
            break;
    }
}
