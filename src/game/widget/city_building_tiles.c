// Buildings in the city view under an overlay that hides them: their flat overlay tiles, or the building
// itself when the overlay shows it. Also the problems overlay's evaluation of a building (its problem bits and
// whether it shows under that overlay).
//
// The problems overlay and the shipwright behave a little oddly:
// - a shipwright without an order asks shipwright_boat_needed with a slot holding its own building id;
// - a building without laborers keeps its problems overlay mark as it was;
// - the problems draw of a large building looks its main part up and does not use it.

#include "widget/city_building_tiles.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_evolve_checks.h"
#include "building/pyramid_images.h"
#include "building/shipwright.h"
#include "city/finance.h"
#include "city/labor.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "game/game_internal.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/grid.h"
#include "map/terrain.h"
#include "widget/city_view_rotate.h"
#include "widget/city_with_overlay.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include <stdint.h>

enum problem_threshold {
    PROBLEM_RISK_THRESHOLD = 800,
    PROBLEM_PLAGUE_NEARBY_DISTANCE = 3,
};

enum temple_complex_footprint {
    TEMPLE_COMPLEX_LENGTH = 13,
    TEMPLE_COMPLEX_WIDTH = 7,
    TEMPLE_COMPLEX_PART_STEP = 5,
};

enum storage_yard_shift {
    STORAGE_YARD_SHIFT_X = 60,
    STORAGE_YARD_SHIFT_Y = 30,
};

int32_t city_building_cart_pusher_waiting(const building *b)
{
    if (b->figure_id == 0) {
        return 0;
    }
    const figure *f = &city_figures.records[b->figure_id];
    return f->type == FIGURE_CART_PUSHER && f->action_state == FIGURE_ACTION_CART_INITIAL && f->min_max_seen != 0;
}

int32_t city_building_needs_stock(int16_t type)
{
    switch (type) {
        case BUILDING_SENET_HOUSE:
        case BUILDING_MORTUARY:
        case BUILDING_SCRIBAL_SCHOOL:
        case BUILDING_LIBRARY:
        case BUILDING_SHIPWRIGHT:
        case BUILDING_JEWELER:
        case BUILDING_CARPENTERS_GUILD:
        case BUILDING_BRICKLAYERS_GUILD:
        case BUILDING_VILLAGE_PALACE_UP:
        case BUILDING_TOWN_PALACE_UP:
        case BUILDING_CITY_PALACE:
        case BUILDING_CATTLE_RANCH:
        case BUILDING_PAPYRUS_MAKER:
        case BUILDING_BRICKWORKS:
        case BUILDING_CHARIOT_MAKER:
        case BUILDING_ZOO:
        case BUILDING_ARTISANS_GUILD:
        case BUILDING_LAMP_MAKER:
        case BUILDING_PAINT_MAKER:
            return 1;
        default:
            return type >= BUILDING_BREWERY && type <= BUILDING_POTTER;
    }
}

static int32_t shipwright_lacks_stock(int32_t building_id, const building *b)
{
    uint16_t order = b->data.shipyard.boat_order;
    if ((order & BOAT_ORDER_KIND_MASK) == 0) {
        int32_t boat_type = building_id;
        return shipwright_boat_needed(&boat_type) != 0;
    }
    if ((order & BOAT_ORDER_KIND_MASK) == BOAT_ORDER_FOR_CITY && (order & BOAT_ORDER_TYPE_MASK) == FIGURE_FISHING_BOAT) {
        return 0;
    }
    return 1;
}

static void update_house_problems(int32_t building_id, building *b)
{
    if (b->state == BUILDING_STATE_UNUSED) {
        return;
    }
    if (b->house_population == 0) {
        b->problem_flags |= BUILDING_PROBLEM_NO_PEOPLE;
    } else if (b->house_plague_distance != 0 && b->house_plague_distance < PROBLEM_PLAGUE_NEARBY_DISTANCE) {
        b->problem_flags |= BUILDING_PROBLEM_PLAGUE_NEARBY;
    }
    if (b->data.house.is_sick == HOUSE_SICK_DISEASE) {
        b->problem_flags |= BUILDING_PROBLEM_DISEASE;
    } else if (b->data.house.is_sick == HOUSE_SICK_FROGS) {
        b->problem_flags |= BUILDING_PROBLEM_FROGS;
    } else if (b->house_plague_risk > PROBLEM_RISK_THRESHOLD) {
        if (b->house_population == 0) {
            return;
        }
        b->problem_flags |= BUILDING_PROBLEM_MALARIA_RISK;
    }
    if (b->house_population == 0) {
        return;
    }
    if (b->crime_risk > PROBLEM_RISK_THRESHOLD) {
        b->problem_flags |= BUILDING_PROBLEM_CRIME_RISK;
    }
    house_update_evolve_text(building_id);
    uint8_t evolve = b->data.house.evolve_text_id;
    if (evolve <= EVOLVE_TEXT_BEER || evolve == EVOLVE_TEXT_JEWELRY || evolve == EVOLVE_TEXT_SECOND_JEWELRY) {
        b->problem_flags |= BUILDING_PROBLEM_DEVOLVING;
    }
}

static int32_t update_work_problems(int32_t building_id, building *b)
{
    int16_t type = b->type;
    if (building_is_farm(building_id) || (type >= BUILDING_STONE_QUARRY && type <= BUILDING_CLAY_PIT)) {
        if (city_building_cart_pusher_waiting(b)) {
            b->show_on_problem_overlay = 1;
        }
    } else if (city_building_needs_stock(type) && building_is_palace_or_mansion(building_id) == 0) {
        if (city_building_cart_pusher_waiting(b)) {
            b->show_on_problem_overlay = 1;
        } else if (b->loads_stored <= 0 && (type != BUILDING_SHIPWRIGHT || shipwright_lacks_stock(building_id, b))) {
            b->problem_flags |= BUILDING_PROBLEM_NO_STOCK;
            b->show_on_problem_overlay = 1;
        }
    }
    int32_t laborers = building_model_table[b->type].laborers;
    if (laborers == 0) {
        return 0;
    }
    if (city_labor_industry_is_mothballed(building_id) != 0) {
        b->problem_flags |= BUILDING_PROBLEM_MOTHBALLED;
    } else if (b->percentage_houses_covered <= 0 && building_is_floodplain_farm(building_id) == 0) {
        b->problem_flags |= BUILDING_PROBLEM_NO_LABOR_ACCESS;
    } else if (b->num_workers == 0) {
        b->problem_flags |= BUILDING_PROBLEM_NO_WORKERS;
    } else if (b->num_workers < laborers) {
        b->problem_flags |= BUILDING_PROBLEM_FEW_WORKERS;
    }
    return 1;
}

void city_building_update_problems(int32_t building_id)
{
    building *b = building_get(building_id);
    if (b->type == BUILDING_WELL) {
        b->problem_flags = 0;
        return;
    }
    if (building_id == 0 || b->prev_part_building_id != 0) {
        return;
    }
    b->problem_flags = 0;
    if (b->damage_risk > PROBLEM_RISK_THRESHOLD) {
        b->problem_flags = BUILDING_PROBLEM_DAMAGE_RISK;
    }
    if (b->fire_risk > PROBLEM_RISK_THRESHOLD) {
        b->problem_flags |= BUILDING_PROBLEM_FIRE_RISK;
    }
    if (b->house_size != 0) {
        update_house_problems(building_id, b);
    } else if (!update_work_problems(building_id, b)) {
        return;
    }
    if (b->problem_flags != 0 && b->show_on_problem_overlay == 0) {
        b->show_on_problem_overlay = 1;
    }
}

static void draw_image_at_size(int32_t image, int32_t x, int32_t y)
{
    switch (map_grids.bitfields[city_view.grid_offset] & MAP_BITFIELD_SIZE) {
        case 1:
            image_draw(image, city_view.tile_half_width + 1 + x, y - city_view.tile_half_height);
            return;
        case 2:
            image_draw(image, city_view.tile_width + 2 + x, y - city_view.tile_height);
            return;
        case 3:
            image_draw(image, city_view.size3_x + 3 + x, y - city_view.size3_y);
            return;
        case 4:
            image_draw(image, city_view.size4_x + 4 + x, y - city_view.size4_y);
            return;
        case 5:
            image_draw(image, city_view.size4_x + city_view.tile_half_width + 5 + x, (y - city_view.size4_y) - city_view.tile_half_height);
            return;
        default:
            image_draw(image, x, y);
            return;
    }
}

static void draw_overlay_tiles(const int32_t *offsets, const int32_t *images, int32_t count, int32_t house_record, int32_t x, int32_t y, int32_t flags)
{
    for (int32_t i = 0; i < count; i++) {
        g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY) + images[i] + flags;
        if (building_get(house_record)->house_size != 0) {
            g_last_image_id += OVERLAY_TILE_HOUSE;
        }
        image_draw(g_last_image_id, offsets[i * 2] + x, offsets[(i * 2) + 1] + y);
    }
}

static int32_t big_building_tile(int32_t r, int32_t c, int32_t rows, int32_t columns)
{
    switch (map_info.orientation) {
        case VIEW_ORIENTATION_NORTH:
            if (r == 0) {
                return (c == 0) ? OVERLAY_TILE_CORNER : ((c > 0) ? OVERLAY_TILE_EDGE_A : OVERLAY_TILE_INNER);
            }
            return (r > 0 && c == 0) ? OVERLAY_TILE_EDGE_B : OVERLAY_TILE_INNER;
        case VIEW_ORIENTATION_EAST:
            if (r == (rows - 1) && c == 0) {
                return OVERLAY_TILE_CORNER;
            }
            if (r == (rows - 1) && c > 0) {
                return OVERLAY_TILE_EDGE_B;
            }
            return (r > 0 && c == 0) ? OVERLAY_TILE_EDGE_A : OVERLAY_TILE_INNER;
        case VIEW_ORIENTATION_SOUTH:
            if (r == (rows - 1) && c == (columns - 1)) {
                return OVERLAY_TILE_CORNER;
            }
            if (r == (rows - 1) && c >= 0) {
                return OVERLAY_TILE_EDGE_A;
            }
            return (r >= 0 && c == (columns - 1)) ? OVERLAY_TILE_EDGE_B : OVERLAY_TILE_INNER;
        default:
            if (r == 0 && c == (columns - 1)) {
                return OVERLAY_TILE_CORNER;
            }
            if (r == 0 && c >= 0) {
                return OVERLAY_TILE_EDGE_B;
            }
            return (r >= 0 && c == (columns - 1)) ? OVERLAY_TILE_EDGE_A : OVERLAY_TILE_INNER;
    }
}

static void draw_temple_complex_tiles(const building *b)
{
    uint8_t orientation = b->data.temple_complex.orientation;
    const building_part_offset *offset = &building_part_offsets[orientation >> 1];
    int32_t base_x = b->x + offset->x;
    int32_t base_y = b->y + offset->y;
    int32_t rows, columns, step;
    switch (orientation) {
        case 0:
            rows = TEMPLE_COMPLEX_LENGTH;
            columns = TEMPLE_COMPLEX_WIDTH;
            step = 1;
            break;
        case 2:
            rows = TEMPLE_COMPLEX_WIDTH;
            columns = TEMPLE_COMPLEX_LENGTH;
            step = TEMPLE_COMPLEX_PART_STEP;
            break;
        case 4:
            rows = TEMPLE_COMPLEX_LENGTH;
            columns = TEMPLE_COMPLEX_WIDTH;
            step = TEMPLE_COMPLEX_PART_STEP;
            break;
        case 6:
            rows = TEMPLE_COMPLEX_WIDTH;
            columns = TEMPLE_COMPLEX_LENGTH;
            step = 1;
            break;
        default:
            return;
    }
    for (int32_t r = 0; r < rows; r++) {
        for (int32_t c = 0; c < columns; c++) {
            int32_t tile = big_building_tile(r, c, rows, columns);
            int32_t px, py;
            city_view_tile_to_pixel((r * step) + base_x, (c * step) + base_y, &px, &py);
            g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY) + tile;
            image_draw(g_last_image_id, px, py);
        }
    }
}

static void draw_storage_yard_tiles(int32_t building_id, int32_t x, int32_t y)
{
    if (map_info.orientation == VIEW_ORIENTATION_NORTH) {
        x -= STORAGE_YARD_SHIFT_X;
        y += STORAGE_YARD_SHIFT_Y;
    } else if (map_info.orientation == VIEW_ORIENTATION_SOUTH) {
        x -= STORAGE_YARD_SHIFT_X;
        y -= STORAGE_YARD_SHIFT_Y;
    } else if (map_info.orientation == VIEW_ORIENTATION_WEST) {
        x -= 2 * STORAGE_YARD_SHIFT_X;
    }
    draw_overlay_tiles(tile_grid_offsets_base.size3_pixels, overlay_tiles_size3, 9, building_id, x, y, 0);
}

static void draw_single_tile(int32_t building_id, const building *b, int32_t x, int32_t y, int32_t flags)
{
    g_last_image_id = image_group(IMAGE_GROUP_TERRAIN_OVERLAY);
    int32_t image = image_group(IMAGE_GROUP_TERRAIN_OVERLAY);
    if (b->house_size != 0) {
        if (city_overlay == OVERLAY_PROBLEMS && b->show_on_problem_overlay != 0) {
            image = map_grids.images[city_view.grid_offset];
        } else {
            image += OVERLAY_TILE_HOUSE;
        }
        image += flags;
    } else if (city_overlay == OVERLAY_PROBLEMS && building_get(building_main(building_id))->problem_flags != 0) {
        image = map_grids.images[city_view.grid_offset];
    } else {
        image = g_last_image_id + flags;
    }
    g_last_image_id = image;
    image_draw(image, x, y);
}

static void draw_small_footprint(int32_t building_id, int32_t size, int32_t x, int32_t y, int32_t flags)
{
    int32_t house_record = building_id;
    if (city_overlay == OVERLAY_PROBLEMS) {
        house_record = building_main(building_id);
        if (building_get(house_record)->problem_flags != 0) {
            g_last_image_id = map_grids.images[city_view.grid_offset];
            if (size == 1) {
                image_draw(g_last_image_id, city_view.tile_half_width + 1 + x, y - city_view.tile_half_height);
            } else {
                image_draw(g_last_image_id, city_view.tile_width + 2 + x, y - city_view.tile_height);
            }
            return;
        }
    }
    if (size == 1) {
        draw_overlay_tiles(tile_grid_offsets_base.size2_pixels, overlay_tiles_size2, 4, house_record, x, y, flags);
    } else {
        draw_overlay_tiles(tile_grid_offsets_base.size3_pixels, overlay_tiles_size3, 9, house_record, x, y, flags);
    }
}

void city_building_draw_tiles(int32_t building_id, int32_t x, int32_t y, int32_t flags)
{
    if (building_id == 0) {
        return;
    }
    if (city_overlay == OVERLAY_PROBLEMS) {
        city_building_update_problems(building_id);
    }
    int32_t main_id = building_main(building_id);
    const building *b = building_get(building_id);
    int32_t main_has_problems = building_get(main_id)->problem_flags != 0 && city_overlay == OVERLAY_PROBLEMS;
    if (city_with_overlay_show_building(main_id) != 0 || b->type == BUILDING_FESTIVAL_SQUARE || main_has_problems) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        draw_image_at_size(g_last_image_id, x, y);
        return;
    }
    int16_t type = b->type;
    int32_t size = map_grids.bitfields[city_view.grid_offset] & MAP_BITFIELD_SIZE;
    if ((type == BUILDING_BOOTH || type == BUILDING_BANDSTAND || type == BUILDING_PAVILION) &&
        (map_grids.terrain[city_view.grid_offset] & (TERRAIN_GARDEN | TERRAIN_ROAD)) != 0) {
        g_last_image_id = map_grids.images[city_view.grid_offset];
        draw_image_at_size(g_last_image_id, x, y);
        return;
    }
    if (type == BUILDING_STORAGE_YARD) {
        draw_storage_yard_tiles(building_id, x, y);
        return;
    }
    if (type == BUILDING_STORAGE_ROOM) {
        return;
    }
    if (type >= BUILDING_TEMPLE_COMPLEX_OSIRIS && type <= BUILDING_TEMPLE_COMPLEX_BAST) {
        if (b->prev_part_building_id == 0) {
            draw_temple_complex_tiles(b);
        }
        return;
    }
    if (size == 0) {
        draw_single_tile(building_id, b, x, y, flags);
        return;
    }
    if (size == 1 || size == 2) {
        draw_small_footprint(building_id, size, x, y, flags);
        return;
    }
    if (size > 5) {
        return;
    }
    if (city_overlay == OVERLAY_PROBLEMS && b->problem_flags != 0) {
        building_main(building_id);
        g_last_image_id = map_grids.images[city_view.grid_offset];
        draw_image_at_size(g_last_image_id, x, y);
        return;
    }
    if (size == 3) {
        draw_overlay_tiles(tile_grid_offsets_base.size4_pixels, overlay_tiles_size4, 16, building_id, x, y, flags);
    } else if (size == 4) {
        draw_overlay_tiles(tile_grid_offsets_base.size5_pixels, overlay_tiles_size5, 25, building_id, x, y, flags);
    } else {
        draw_overlay_tiles(tile_grid_offsets_base.size6_pixels, overlay_tiles_size6, 36, building_id, x, y, flags);
    }
}
