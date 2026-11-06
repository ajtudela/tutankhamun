// The columns the overlays draw on top of buildings, one function per overlay: a house's risk, service,
// education, religion, food or tax value as the column's height, a building's labor coverage, the tax income
// coins and the problems overlay's icons.
//
// The house columns first leave the tile's map image in g_last_image_id, which later draws read.

#include "widget/city_overlay_columns.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_figure_helpers.h"
#include "building/building_image_group.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/house_services.h"
#include "building/pyramid_images.h"
#include "city/resource_type.h"
#include "core/calc.h"
#include "empire/empire.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/farm.h"
#include "map/grid.h"
#include "widget/city_building_anims.h"
#include "widget/city_building_tiles.h"
#include "widget/city_overlay_column.h"
#include "widget/city_overlay_heights.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include <stdint.h>

static void column_risk(int32_t building_id, int32_t x, int32_t y, int16_t risk)
{
    const building *b = building_get(building_id);
    if (b->prev_part_building_id != 0) {
        return;
    }
    if (city_overlay_farm_hidden(building_id)) {
        return;
    }
    int32_t height = risk / 100;
    int32_t px = x;
    int32_t py = y;
    if (b->type > BUILDING_TEMPLE_BAST && b->type < BUILDING_BAZAAR) {
        const building_part_offset *offset = &building_part_offsets[b->data.temple_complex.orientation >> 1];
        city_view_tile_to_pixel(b->x + offset->x, b->y + offset->y, &px, &py);
    }
    city_overlay_draw_column(height, px, py, OVERLAY_COLUMN_RISK);
}
void city_overlay_column_fire(int32_t building_id, int32_t x, int32_t y)
{
    column_risk(building_id, x, y, building_get(building_id)->fire_risk);
}
void city_overlay_column_damage(int32_t building_id, int32_t x, int32_t y)
{
    column_risk(building_id, x, y, building_get(building_id)->damage_risk);
}

void city_overlay_column_disease(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        city_overlay_draw_column(b->house_plague_risk / 100, x, y, OVERLAY_COLUMN_RISK);
    }
}
void city_overlay_column_malaria(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        city_overlay_draw_column(b->house_malaria_risk / 100, x, y, OVERLAY_COLUMN_RISK);
    }
}
void city_overlay_column_crime(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        city_overlay_draw_column(b->crime_risk / 100, x, y, OVERLAY_COLUMN_RISK);
    }
}

void city_overlay_column_entertainment(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        city_overlay_draw_column(b->data.house.entertainment / 10, x, y, OVERLAY_COLUMN_SERVICE);
    }
}

void city_overlay_column_education(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return;
    }
    int32_t height;
    switch (b->data.house.education) {
        case HOUSE_EDUCATION_SCHOOL_OR_LIBRARY:
            height = 2;
            break;
        case HOUSE_EDUCATION_SCHOOL_AND_LIBRARY:
            height = 5;
            break;
        case HOUSE_EDUCATION_HIGHER:
            height = 8;
            break;
        default:
            return;
    }
    city_overlay_draw_column(height, x, y, OVERLAY_COLUMN_SERVICE);
}

static void column_access(int32_t building_id, int32_t x, int32_t y, uint8_t access, int32_t style)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    if (building_get(building_id)->house_size != 0) {
        city_overlay_draw_column(access / 10, x, y, style);
    }
}
void city_overlay_column_booth(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.juggler, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_bandstand(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.musician, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_pavilion(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.dancer, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_senet(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.senet, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_zoo(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.zoo, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_water(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.water_supply, OVERLAY_COLUMN_WATER);
}
void city_overlay_column_school(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.school, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_library(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.library, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_dentist(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.dentist, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_physician(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.physician, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_apothecary(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.apothecary, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_mortuary(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.mortuary, OVERLAY_COLUMN_SERVICE);
}
void city_overlay_column_courthouse(int32_t b, int32_t x, int32_t y)
{
    column_access(b, x, y, building_get(b)->data.house.courthouse, OVERLAY_COLUMN_SERVICE);
}

void city_overlay_column_bazaar_access(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return;
    }
    int32_t needed = 0;
    if (house_models[b->subtype].food_types != 0) {
        needed = b->house_population;
    }
    int32_t sum = 0;
    for (int32_t resource = RESOURCE_MIN_FOOD; resource <= RESOURCE_GAMEMEAT; resource++) {
        if (empire_can_produce_resource(resource) != 0 || empire_can_import_resource_potentially(resource) != 0) {
            sum += b->data.house.inventory[resource - RESOURCE_MIN_FOOD];
        }
    }
    int32_t percent = (needed == 0) ? 0 : calc_percentage(sum, needed);
    city_overlay_draw_column(percent / 100, x, y, OVERLAY_COLUMN_SERVICE);
}

void city_overlay_column_religion(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size != 0) {
        city_overlay_draw_column(b->data.house.num_gods * 17, x, y, OVERLAY_COLUMN_SERVICE);
    }
}

void city_overlay_draw_tax_coins(int32_t value, int32_t x, int32_t y)
{
    if (value >= 200) {
        value = 200;
    }
    int32_t minimum = (value <= 0) ? 0 : 3;
    if (value <= minimum) {
        value = minimum;
    }
    int32_t coins = value / 3;
    y += 10;
    while (coins > 0) {
        int32_t image = (coins < 4) ? ((coins % 4) - 1) : 3;
        image_draw(image_group(IMAGE_GROUP_TAX_COINS) + image, x + 15, y);
        coins -= 4;
        y -= 12;
    }
}

void city_overlay_column_tax_income(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return;
    }
    city_overlay_draw_column((b->house_tax_coverage * 20) / 100, x, y, OVERLAY_COLUMN_SERVICE);
    city_overlay_draw_tax_coins(building_house_tax_income(building_id), x, y);
}

void city_overlay_column_problems(int32_t building_id, int32_t x, int32_t y)
{
    int32_t main_id = building_main(building_id);
    const building *main = building_get(main_id);
    if (main->problem_flags == 0) {
        return;
    }
    uint8_t bits = map_grids.bitfields[city_view.grid_offset];
    g_last_image_id = map_grids.images[city_view.grid_offset];
    int32_t size = bits & MAP_BITFIELD_SIZE;
    int32_t draw_x = x;
    int32_t draw_y = y;
    int32_t draw = 1;
    switch (size) {
        case 0:
            break;
        case 1:
            draw_x = city_view.tile_half_width + 1 + x;
            draw_y = y - city_view.tile_half_height;
            break;
        case 2:
            draw_x = city_view.tile_width + 2 + x;
            draw_y = y - city_view.tile_height;
            break;
        case 3:
            draw_x = city_view.size3_x + 3 + x;
            draw_y = y - city_view.size3_y;
            break;
        case 4:
            draw_x = city_view.size4_x + 4 + x;
            draw_y = y - city_view.size4_y;
            break;
        case 5:
            draw_x = city_view.size4_x + city_view.tile_half_width + 5 + x;
            draw_y = (y - city_view.size4_y) - city_view.tile_half_height;
            break;
        default:
            draw = 0;
            break;
    }
    if (draw) {
        image_draw_isometric_top(g_last_image_id, draw_x, draw_y);
    }
    const building *b = building_get(building_id);
    const building *icons = b;
    if (b->house_size == 0) {
        if (b->type == BUILDING_GRANARY) {
            draw_granary_stores(building_id, city_view.grid_offset, x, y);
        } else if (building_is_farm(building_id)) {
            if (map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) {
                map_building_tiles_add_farm(building_id, city_view.tile_width + 2 + x, y - city_view.tile_height, 1);
            }
        } else if (main->type > BUILDING_TEMPLE_BAST && main->type < BUILDING_BAZAAR) {
            if (size == 0 || (map_grids.edge[city_view.grid_offset] & MAP_EDGE_DRAW_TILE) == 0) {
                return;
            }
            icons = main;
        }
    }
    int32_t icon_x = (city_view.tile_half_width * size) + 18 + x;
    int32_t icon_y = (city_view.tile_half_height * size) - 2 + y;
    for (int32_t bit = 0; bit < BUILDING_PROBLEM_BITS; bit++) {
        int32_t icon = (bit == BUILDING_PROBLEM_BIT_NO_LABOR_ACCESS) ? BUILDING_PROBLEM_BIT_NO_WORKERS : bit;
        if (icons->problem_flags & (1 << bit)) {
            image_draw(image_group(IMAGE_GROUP_PROBLEM_ICONS) + icon, icon_x, icon_y);
            icon_y -= 14;
        }
    }
}

void city_overlay_column_infected_housing(int32_t building_id, int32_t x, int32_t y)
{
    g_last_image_id = map_grids.images[city_view.grid_offset];
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        return;
    }
    int32_t flag_x = ((b->size - 1) * city_view.tile_half_width) + 18 + x;
    if (b->data.house.is_sick == HOUSE_SICK_DISEASE) {
        image_draw(image_group(IMAGE_GROUP_FARM_FLAG), flag_x, y);
    } else if (b->data.house.is_sick == HOUSE_SICK_FROGS) {
        image_draw(image_group(IMAGE_GROUP_FARM_FLAG_LATE), flag_x, y);
    }
}

void city_overlay_column_labor(int32_t building_id, int32_t x, int32_t y)
{
    const building *b = building_get(building_id);
    if (b->house_size == 0) {
        city_overlay_draw_column(calc_percentage(b->houses_covered, 300), x, y, OVERLAY_COLUMN_SERVICE);
    }
}
