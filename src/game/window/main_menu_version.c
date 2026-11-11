// The version line of the front menu, bottom left in the small font as in Akhenaten ("version_number" of
// ui_main_menu.js).

#include "window/main_menu_version.h"
#include "graphics/color.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/text_draw.h"
#include "platform/version.h"
#include "window/window_image_group.h"

enum main_menu_version_layout {
    VERSION_X = 16,
    VERSION_BOTTOM = 20,
};

void main_menu_version_draw(void)
{
    text_draw((const uint8_t *) version_text(), VERSION_X, g_screen_height - VERSION_BOTTOM, image_group(IMAGE_GROUP_FONT_SMALL), COLOR_WHITE);
}
