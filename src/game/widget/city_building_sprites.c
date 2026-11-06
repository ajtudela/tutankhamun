// Two sprite helpers of the city drawing's top pass: an image drawn at an offset from its sprite origin, and
// the stock piles drawn on workshops and guilds. They only draw.
//
// Pile and offset lookups:
// - the pile shown is (loads - 100) / 100 rounded toward zero, so 1..199 loads show the first pile;
// - an image id past the image table takes image 0's sprite offsets.

#include "widget/city_building_sprites.h"
#include "building/building.h"
#include "building/building_data.h"
#include "building/building_image_group.h"
#include "building/building_type.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include <stdint.h>
#include <string.h>

int32_t image_draw_sprite_offset(int32_t image_id, int32_t x, int32_t y, int32_t dx, int32_t dy)
{
    int32_t ox = image_record_sprite_offset_x(image_id < image_collection.image_count ? image_id : 0);
    int32_t oy = image_record_sprite_offset_y(image_id < image_collection.image_count ? image_id : 0);
    return image_draw(image_id, x + (dx - ox), (y - oy) + dy);
}

static void draw_pile(int32_t amount, int32_t max, int32_t group, int32_t x, int32_t y)
{
    if (amount >= 0) {
        if (amount > max) {
            amount = max;
        }
        image_draw(amount + image_group(group), x, y);
    }
}

void draw_workshop_raw_material_storage(int32_t building_id, int32_t x, int32_t y)
{
    int32_t a = (city_buildings.records[building_id].loads_stored - 100) / 100;
    int32_t b = (city_buildings.records[building_id].second_input_stock - 100) / 100;
    switch (city_buildings.records[building_id].type) {
        case BUILDING_SENET_HOUSE:
            draw_pile(a, 2, IMAGE_GROUP_STOCK_BEER, x + 107, y + 5);
            break;
        case BUILDING_MORTUARY:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_LINEN, x + 49, y + 16);
            break;
        case BUILDING_SCRIBAL_SCHOOL:
            draw_pile(a, 2, IMAGE_GROUP_STOCK_PAPYRUS, x + 28, y + 15);
            break;
        case BUILDING_LIBRARY:
            draw_pile(a, 2, IMAGE_GROUP_STOCK_PAPYRUS, x + 61, y - 13);
            break;
        case BUILDING_SHIPWRIGHT:
            if (a >= 0) {
                static const int32_t offsets[4][2] = { { 0x46, 0x19 }, { 0x78, -0x19 }, { 0x5A, 10 }, { 0x28, -5 } };
                int32_t view = city_buildings.records[building_id].data.oriented.orientation - map_info.orientation / 2;
                if (view < 0) {
                    view += 4;
                }
                if (a > 4) {
                    a = 4;
                }
                image_draw(a + image_group(IMAGE_GROUP_STOCK_WOOD), offsets[view][0] + x, offsets[view][1] + y);
            }
            break;
        case BUILDING_RECRUITER:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_WEAPONS, x + 131, y + 2);
            draw_pile(b, 1, IMAGE_GROUP_STOCK_CHARIOTS, x + 74, y + 10);
            break;
        case BUILDING_WOOD_CUTTER:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_WOOD, x + 65, y + 3);
            break;
        case BUILDING_BREWERY:
            draw_pile(a, 2, IMAGE_GROUP_STOCK_BARLEY, x + 43, y - 1);
            break;
        case BUILDING_WEAVER:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_FLAX, x + 41, y + 1);
            break;
        case BUILDING_WEAPONSMITH:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_COPPER, x + 69, y + 19);
            break;
        case BUILDING_JEWELER:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_GEMS, x + 63, y - 1);
            break;
        case BUILDING_POTTER:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_CLAY, x + 65, y + 20);
            break;
        case BUILDING_HUNTING_LODGE:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_GAME_MEAT, x + 61, y + 13);
            break;
        case BUILDING_CARPENTERS_GUILD:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_WOOD, x + 41, y + 8);
            break;
        case BUILDING_CATTLE_RANCH:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_STRAW, x + 38, y + 7);
            break;
        case BUILDING_REED_GATHERER:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_REEDS, x + 34, y + 17);
            break;
        case BUILDING_PAPYRUS_MAKER:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_REEDS, x + 35, y + 4);
            break;
        case BUILDING_BRICKWORKS:
            draw_pile(b, 1, IMAGE_GROUP_STOCK_STRAW, x + 91, y + 2);
            draw_pile(a, 1, IMAGE_GROUP_STOCK_CLAY, x + 52, y - 17);
            break;
        case BUILDING_CHARIOT_MAKER:
            draw_pile(a, 4, IMAGE_GROUP_STOCK_WOOD, x + 166, y - 23);
            break;
        case BUILDING_ZOO:
            draw_pile(a, 2, IMAGE_GROUP_STOCK_GAME_MEAT, x + 200, y - 10);
            draw_pile(b, 2, IMAGE_GROUP_STOCK_STRAW, x + 220, y + 10);
            break;
        case BUILDING_ARTISANS_GUILD:
            draw_pile(b, 4, IMAGE_GROUP_STOCK_CLAY, x + 1, y + 7);
            draw_pile(a, 4, IMAGE_GROUP_STOCK_ARTISANS_GUILD, x + 46, y + 12);
            break;
        case BUILDING_LAMP_MAKER:
            draw_pile(b, 1, IMAGE_GROUP_STOCK_LAMP_MAKER_SECOND_INPUT, x + 50, y + 20);
            draw_pile(a, 1, IMAGE_GROUP_STOCK_LAMP_MAKER, x + 15, y + 8);
            break;
        case BUILDING_PAINT_MAKER:
            draw_pile(a, 1, IMAGE_GROUP_STOCK_PAINT_MAKER, x + 45, y + 25);
            break;
        default:
            break;
    }
}
