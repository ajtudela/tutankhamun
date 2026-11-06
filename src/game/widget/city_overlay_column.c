// One overlay column over a building: a base, up to nine 10-pixel segments and a top, in the service, risk or
// water style (the risk column darkens as it grows).
//
// The column leaves the base or the segment image in g_last_image_id.

#include "widget/city_overlay_column.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "widget/widget_data.h"
#include "widget/widget_image_group.h"
#include <stdint.h>

enum overlay_column_shade {
    COLUMN_SHADE_NORMAL = 0,
    COLUMN_SHADE_RISK_MEDIUM = 3,
    COLUMN_SHADE_RISK_HIGH = 6,
    COLUMN_SHADE_RISK_SEVERE = 9,
    COLUMN_SHADE_WATER = 12,
};

enum overlay_column_part {
    COLUMN_PART_TOP = 0,
    COLUMN_PART_SEGMENT = 1,
    COLUMN_PART_BASE = 2,
};

enum overlay_column_height {
    COLUMN_HEIGHT_MAX = 10,
    COLUMN_HEIGHT_RISK_SEVERE = 9,
    COLUMN_HEIGHT_RISK_HIGH = 7,
    COLUMN_HEIGHT_RISK_MEDIUM = 5,
    COLUMN_SEGMENT_PIXELS = 10,
};

void city_overlay_draw_column(int32_t height, int32_t x, int32_t y, int32_t column_style)
{
    int32_t h = height;
    if (h > COLUMN_HEIGHT_MAX) {
        h = COLUMN_HEIGHT_MAX;
    }
    int32_t shade = COLUMN_SHADE_NORMAL;
    if (column_style == OVERLAY_COLUMN_WATER) {
        shade = COLUMN_SHADE_WATER;
    } else if (column_style != OVERLAY_COLUMN_SERVICE) {
        if (h >= COLUMN_HEIGHT_RISK_SEVERE) {
            shade = COLUMN_SHADE_RISK_SEVERE;
        } else if (h >= COLUMN_HEIGHT_RISK_HIGH) {
            shade = COLUMN_SHADE_RISK_HIGH;
        } else if (h >= COLUMN_HEIGHT_RISK_MEDIUM) {
            shade = COLUMN_SHADE_RISK_MEDIUM;
        }
    }
    int32_t base = image_group(IMAGE_GROUP_OVERLAY_COLUMN) + shade;
    int32_t img = image_known_id(base);
    int32_t capital_height = image_record_height(img);
    if (h == 0) {
        g_last_image_id = base + COLUMN_PART_BASE;
        image_draw(g_last_image_id, x + 9, y - 8);
        return;
    }
    if (h == 1) {
        image_draw(base + COLUMN_PART_BASE, x + 9, y - 8);
        image_draw(base + COLUMN_PART_TOP, x + 5, y - capital_height + 5);
        return;
    }
    image_draw(base + COLUMN_PART_BASE, x + 9, y - 8);
    image_draw(base + COLUMN_PART_TOP, x + 5, (y + (h * -COLUMN_SEGMENT_PIXELS)) - capital_height + 15);
    g_last_image_id = image_group(IMAGE_GROUP_OVERLAY_COLUMN) + COLUMN_PART_SEGMENT + shade;
    int32_t py = y - 5;
    for (int32_t segments = h - 1; segments > 0; segments--) {
        image_draw(g_last_image_id, x + 17, py);
        py -= COLUMN_SEGMENT_PIXELS;
    }
}
