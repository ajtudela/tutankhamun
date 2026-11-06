// Figures drawn in the city view: a figure at its place on the tile (progress or cross-country offset, raised
// by its height), with its cart, wheels, flag, standard, shadow or sail drawn before or after it; all figures on
// the view's tiles; and the city drawn around one figure for the building info window's figure view.

#include "widget/city_figure.h"
#include "building/building_data.h"
#include "building/pyramid_images.h"
#include "figure/figure.h"
#include "figure/figure_data.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "game/time.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/text_draw.h"
#include "map/grid.h"
#include "widget/city_view.h"
#include "widget/city_view_rotate.h"
#include "widget/city_with_overlay.h"
#include "widget/city_without_overlay.h"
#include "widget/widget_data.h"
#include "window/window_image_group.h"
#include <stdint.h>

int32_t g_skip_draw_figures;

enum figure_draw_layout {
    FIGURE_PIXEL_X = 2,
    FIGURE_BASE_Y = 10,
    FIGURE_TALL_BASE_Y = 12,
    FIGURE_ROAD_RAISE_Y = 5,
    FIGURE_CROSS_COUNTRY_TILE = 15,
    FIGURE_CART_RAISE_Y = 10,
    BALLISTA_JAVELIN_RAISE_Y = 18,
    MISSILE_SHADOW_Y = 10,
    SHIP_SAIL_RAISE_Y = 24,
    STANDARD_FLAG_FRAMES = 20,
    STANDARD_EXPERIENCE_PER_FRAME = 5,
    MAP_FLAG_NUMBER_X = 9,
    MAP_FLAG_NUMBER_Y = 14,
    MAP_FLAG_NUMBER_PREFIX = ' ',
    MAP_FLAG_NUMBER_FONT = 0x6B,
    DIRECTIONS = 8,
    CART_AFTER_FIGURE_FACING = DIRECTION_BOTTOM_RIGHT,
};

enum map_flag_range {
    MAP_FLAG_POINTS_FIRST = 0x04,
    MAP_FLAG_POINTS_LAST = 0x0C,
    MAP_FLAG_FISHING_FIRST = 0x0E,
    MAP_FLAG_FISHING_LAST = 0x15,
    MAP_FLAG_PREDATOR_HERD_FIRST = 0x16,
    MAP_FLAG_PREDATOR_HERD_LAST = 0x19,
    MAP_FLAG_PREY_HERD_FIRST = 0x1A,
    MAP_FLAG_PREY_HERD_LAST = 0x1D,
    MAP_FLAG_DISEMBARK_FIRST = 0x1E,
    MAP_FLAG_DISEMBARK_LAST = 0x20,
    MAP_FLAG_INVASION_FIRST = 0x32,
    MAP_FLAG_INVASION_LAST = 0x41,
};

enum figures_pass_layout {
    FIGURES_ROWS_BEFORE = 8,
    FIGURES_ROWS_EXTRA = 14,
    FIGURES_COLS_BEFORE = 4,
    FIGURES_COLS_EXTRA = 6,
    FIGURES_TOP_HALF_TILES = 9,
    TILE_GAP_X = 2,
};

static int32_t image_sprite_offset_x(int32_t image_id)
{
    return image_record_sprite_offset_x(image_known_id(image_id));
}

static int32_t image_sprite_offset_y(int32_t image_id)
{
    return image_record_sprite_offset_y(image_known_id(image_id));
}

static int32_t image_height(int32_t image_id)
{
    return image_record_height(image_known_id(image_id));
}

static int32_t figure_has_tall_base(uint8_t type)
{
    switch (type) {
        case FIGURE_IMMIGRANT:
        case FIGURE_ARCHER:
        case FIGURE_INFANTRY:
        case FIGURE_CHARIOTEER:
        case FIGURE_ENEMY_ARCHER:
        case FIGURE_ENEMY_INFANTRY:
        case FIGURE_ENEMY_CHARIOT:
        case FIGURE_ENEMY_EGYPTIAN_INFANTRY:
        case FIGURE_ENEMY_EGYPTIAN_ARCHER:
        case FIGURE_ENEMY_EGYPTIAN_CHARIOT:
        case FIGURE_CROCODILE:
        case FIGURE_HYENA:
        case FIGURE_HIPPO:
        case FIGURE_BEDOUIN:
        case FIGURE_ASP:
        case FIGURE_LION:
        case FIGURE_SCORPION:
        case FIGURE_FROG:
        case FIGURE_MUMMY:
            return 1;
        default:
            return 0;
    }
}

static int32_t figure_base_y(const figure *f)
{
    if (!figure_has_tall_base(f->type)) {
        return FIGURE_BASE_Y;
    }
    return FIGURE_TALL_BASE_Y + (f->is_on_road ? FIGURE_ROAD_RAISE_Y : 0);
}

static int32_t map_flag_number(uint8_t flag)
{
    if (flag >= MAP_FLAG_POINTS_FIRST && flag <= MAP_FLAG_POINTS_LAST) {
        return flag - MAP_FLAG_POINTS_FIRST + 1;
    }
    if (flag >= MAP_FLAG_FISHING_FIRST && flag <= MAP_FLAG_FISHING_LAST) {
        return flag - MAP_FLAG_FISHING_FIRST + 1;
    }
    if (flag >= MAP_FLAG_PREDATOR_HERD_FIRST && flag <= MAP_FLAG_PREDATOR_HERD_LAST) {
        return flag - MAP_FLAG_PREDATOR_HERD_FIRST + 1;
    }
    if (flag >= MAP_FLAG_PREY_HERD_FIRST && flag <= MAP_FLAG_PREY_HERD_LAST) {
        return flag - MAP_FLAG_PREY_HERD_FIRST + 1;
    }
    if (flag >= MAP_FLAG_DISEMBARK_FIRST && flag <= MAP_FLAG_DISEMBARK_LAST) {
        return flag - MAP_FLAG_DISEMBARK_FIRST + 1;
    }
    if (flag >= MAP_FLAG_INVASION_FIRST && flag <= MAP_FLAG_INVASION_LAST) {
        return flag - MAP_FLAG_INVASION_FIRST + 1;
    }
    return 0;
}

static int32_t figure_draw_direction(const figure *f)
{
    int32_t direction = f->direction;
    if (direction >= DIRECTIONS) {
        direction = f->previous_tile_direction;
        if (direction >= DIRECTIONS) {
            direction = DIRECTION_TOP;
        }
    }
    return direction;
}

static int32_t has_cart_behind(uint8_t type)
{
    return type == FIGURE_CART_PUSHER || type == FIGURE_DOCKER || type == FIGURE_STORAGE_YARD_CART;
}

static int32_t has_cart_beside(uint8_t type)
{
    return type == FIGURE_NATIVE_TRADER || type == FIGURE_CHARIOTEER || type == FIGURE_ENEMY_EGYPTIAN_CHARIOT || type == FIGURE_ENEMY_CHARIOT ||
        type == FIGURE_IMMIGRANT || type == FIGURE_EMIGRANT || type == FIGURE_FROG;
}

static int32_t is_missile_with_shadow(uint8_t type)
{
    return type == FIGURE_HUNTER_SPEAR || type == FIGURE_JAVELIN || type == FIGURE_SPEAR;
}

static void draw_standard_bearer(const figure *f, int32_t image_id, int32_t cart_image_id, int32_t x, int32_t y)
{
    const formation *company = formation_get(f->formation_id);
    if (company->request_event_id >= 0) {
        return;
    }
    int32_t icon = company->legion_id + image_group(IMAGE_GROUP_FORT_LEGION_STANDARDS);
    if (company->ship_figure_id != 0) {
        image_draw(icon, x, y - image_height(cart_image_id));
        return;
    }
    image_draw(image_id, x, y);
    image_draw(image_group(IMAGE_GROUP_FORT_FLAG) - ((company->experience + 3) / STANDARD_EXPERIENCE_PER_FRAME) + STANDARD_FLAG_FRAMES, x, y);
    image_draw(cart_image_id, x, y - image_height(cart_image_id));
    int32_t standard_height = image_height(cart_image_id);
    image_draw(icon, x, y - image_height(icon) - standard_height);
}

static void draw_map_flag(const figure *f, int32_t image_id, int32_t cart_image_id, int32_t x, int32_t y)
{
    if (f->x < 0 || f->y < 0) {
        return;
    }
    if ((f->animation_frame & 1) != 0) {
        y -= 1;
    }
    image_draw(image_id, x, y);
    image_draw(cart_image_id, x, y - image_height(cart_image_id));
    int32_t number = map_flag_number(f->resource_id);
    if (number == 0) {
        return;
    }
    text_draw_number(number, MAP_FLAG_NUMBER_PREFIX, postfix_none, x + MAP_FLAG_NUMBER_X, y + MAP_FLAG_NUMBER_Y, MAP_FLAG_NUMBER_FONT, 0);
}

void city_draw_figure(int32_t figure_id, int32_t x, int32_t y, int32_t highlight)
{
    const figure *f = figure_get(figure_id);
    int32_t cart_image_id = f->cart_image_id;
    int32_t px = FIGURE_PIXEL_X;
    int32_t py = figure_base_y(f);
    figure_image_direction = figure_draw_direction(f) - map_info.orientation;
    if (figure_image_direction < 0) {
        figure_image_direction += DIRECTIONS;
    }
    int32_t image_id = f->image_id;
    if (g_skip_draw_figures == 1) {
        return;
    }

    if (f->use_cross_country == 0) {
        if (f->pyramid_walk_mode < 1) {
            int32_t step = (figure_image_direction * FIGURE_PROGRESS_STEPS) + f->progress_on_tile;
            px = tbl_progress_x[step] + FIGURE_PIXEL_X;
            py += tbl_progress_y[step] - f->current_height;
        } else {
            pyramid_figure_get_pixel_offset(figure_id, &x, &y);
        }
    } else {
        int32_t position = (f->cross_country.x % FIGURE_CROSS_COUNTRY_TILE) + ((f->cross_country.y % FIGURE_CROSS_COUNTRY_TILE) * FIGURE_CROSS_COUNTRY_ROWS);
        int32_t orientation = map_info.orientation;
        if (orientation == VIEW_ORIENTATION_NORTH) {
            px = tbl_cc_ori0_x[position] + FIGURE_PIXEL_X;
            py += tbl_cc_ori0_y[position];
        } else if (orientation == VIEW_ORIENTATION_EAST) {
            px = tbl_cc_ori2_x[position] + FIGURE_PIXEL_X;
            py -= tbl_cc_ori2_y[position];
        } else if (orientation == VIEW_ORIENTATION_SOUTH) {
            px = FIGURE_PIXEL_X - tbl_cc_ori0_x[position];
            py -= tbl_cc_ori0_y[position];
        } else if (orientation == VIEW_ORIENTATION_WEST) {
            px = FIGURE_PIXEL_X - tbl_cc_ori2_x[position];
            py += tbl_cc_ori2_y[position];
        }
        py -= f->missile_damage;
        if (f->type == FIGURE_JAVELIN) {
            const figure *source = figure_get(f->building_id);
            if (source->type == FIGURE_BALLISTA) {
                py += -BALLISTA_JAVELIN_RAISE_Y - source->current_height;
            }
        }
    }

    py += city_view.tile_half_height;
    px += city_view.tile_half_width;
    int32_t cart_py = py - FIGURE_CART_RAISE_Y;
    int32_t cart_px = px - FIGURE_PIXEL_X;
    px -= image_sprite_offset_x(image_id);
    py -= image_sprite_offset_y(image_id);
    cart_px -= image_sprite_offset_x(cart_image_id);
    cart_py -= image_sprite_offset_y(cart_image_id);

    if (highlight != FIGURE_HIGHLIGHT_NONE) {
        if (highlight == FIGURE_HIGHLIGHT_OVERLAY) {
            if (city_with_overlay_show_figure(figure_id) == 0) {
                return;
            }
        } else {
            if (figure_id != highlight) {
                return;
            }
            city_view.selected_figure_pixel_x = px + x;
            city_view.selected_figure_pixel_y = py + y;
        }
    }

    int32_t figure_x = px + x;
    int32_t figure_y = py + y;
    if (cart_image_id == 0) {
        image_draw(image_id, figure_x, figure_y);
        return;
    }

    uint8_t type = f->type;
    if (has_cart_behind(type)) {
        int32_t facing = f->direction + map_info.orientation;
        if (facing >= DIRECTIONS) {
            facing -= DIRECTIONS;
        }
        int32_t cart_x = f->cart_offset_x + cart_px + x;
        int32_t cart_y = f->cart_offset_y + cart_py + y;
        if (facing == CART_AFTER_FIGURE_FACING) {
            image_draw(image_id, figure_x, figure_y);
            image_draw(cart_image_id, cart_x, cart_y);
            return;
        }
        image_draw(cart_image_id, cart_x, cart_y);
        image_draw(image_id, figure_x, figure_y);
        return;
    }

    if (has_cart_beside(type)) {
        int32_t cart_x = f->cart_offset_x + figure_x;
        int32_t cart_y = f->cart_offset_y + figure_y;
        if (f->cart_offset_y >= 0) {
            image_draw(image_id, figure_x, figure_y);
            image_draw(cart_image_id, cart_x, cart_y);
            return;
        }
        image_draw(cart_image_id, cart_x, cart_y);
        image_draw(image_id, figure_x, figure_y);
        return;
    }

    if (type == FIGURE_STANDARD_BEARER) {
        draw_standard_bearer(f, image_id, cart_image_id, figure_x, figure_y);
        return;
    }

    if (type == FIGURE_MAP_FLAG) {
        draw_map_flag(f, image_id, cart_image_id, figure_x, figure_y);
        return;
    }

    if (is_missile_with_shadow(type)) {
        image_draw(cart_image_id, figure_x, figure_y + MISSILE_SHADOW_Y);
        image_draw(image_id, figure_x, figure_y);
        return;
    }

    if (type == FIGURE_TRANSPORT_SHIP || type == FIGURE_WARSHIP) {
        image_draw(image_id, figure_x, figure_y);
        image_draw(cart_image_id, f->cart_offset_x + cart_px + x, f->cart_offset_y + cart_py - SHIP_SAIL_RAISE_Y + y);
        return;
    }

    image_draw(image_id, figure_x, figure_y);
}

void city_draw_figures(int32_t highlight)
{
    if (animation_should_advance[ANIMATION_TIMER_FLASH] == 1) {
        building_counters.figure_flash_tick++;
        if (building_counters.figure_flash_tick > 4) {
            building_counters.figure_flash_tick = 0;
        }
    }

    int32_t pixel_y = city_view.origin_y - (city_view.tile_half_height * FIGURES_TOP_HALF_TILES);
    int32_t row = city_view.camera_row - FIGURES_ROWS_BEFORE;
    int32_t odd_row = 0;
    city_view.row = row;
    for (int32_t row_number = 0; row_number < city_view.height_tiles + FIGURES_ROWS_EXTRA; row_number++) {
        if (row >= 0 && row < VIEW_ROWS) {
            int32_t pixel_x = city_view.origin_x;
            if (odd_row) {
                pixel_x = (city_view.origin_x - city_view.tile_half_width) - 1;
            }
            pixel_x -= (city_view.tile_width + TILE_GAP_X) * FIGURES_COLS_BEFORE;
            int32_t col = city_view.camera_col - FIGURES_COLS_BEFORE;
            city_view.row = row;
            for (int32_t col_number = 0; col_number <= city_view.width_tiles + FIGURES_COLS_EXTRA; col_number++) {
                if (col >= 0 && col < VIEW_COLS) {
                    city_view.grid_offset = city_view.lookup[(row * VIEW_COLS) + col];
                    if (city_view.grid_offset < VIEW_LOOKUP_EMPTY) {
                        int32_t figure_id = map_grids.figure_ids[city_view.grid_offset];
                        city_view.col = col;
                        while (figure_id > 0) {
                            const figure *f = figure_get(figure_id);
                            if (f->height_adjusted_ticks != 0) {
                                city_draw_figure(figure_id, pixel_x, pixel_y, highlight);
                                col = city_view.col;
                                row = city_view.row;
                            }
                            figure_id = f->next_figure_id_on_same_tile;
                        }
                    }
                }
                pixel_x += city_view.tile_width + TILE_GAP_X;
                col++;
            }
            city_view.col = city_view.camera_col - FIGURES_COLS_BEFORE;
            for (int32_t col_number = 0; col_number <= city_view.width_tiles + FIGURES_COLS_EXTRA; col_number++) {
                if (city_view.col >= 0 && city_view.col < VIEW_COLS) {
                    if (city_overlay == OVERLAY_NONE || city_overlay == OVERLAY_HIDE_CLIFFS || overlay_show_all != 0) {
                        city_view.grid_offset = city_view.lookup[(row * VIEW_COLS) + city_view.col];
                    }
                }
                city_view.col++;
            }
        }
        odd_row = !odd_row;
        pixel_y += city_view.tile_half_height;
        row++;
        city_view.row = row;
    }
}

void city_draw_figure_in_view(int32_t camera_x, int32_t camera_y, int32_t grid_start, int32_t figure_id)
{
    city_view.camera_col = camera_x;
    city_view.camera_row = camera_y;
    city_view.selected_tile = grid_start;
    g_clip_x0 = city_view.viewport_x;
    g_clip_x1 = city_view.viewport_x1;
    g_clip_y0 = city_view.viewport_y;
    g_clip_y1 = city_view.viewport_y1;
    city_without_overlay_draw_footprint();
    city_without_overlay_draw_top(figure_id);
    city_draw_figures(FIGURE_HIGHLIGHT_NONE);
    g_clip_x0 = 0;
    g_clip_x1 = g_screen_width;
    g_clip_y0 = 0;
    g_clip_y1 = g_screen_height;
}
