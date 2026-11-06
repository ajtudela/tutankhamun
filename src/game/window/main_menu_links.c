// The link buttons of the front menu, as Akhenaten's main menu has them: Discord, GitHub, Patreon and wiki icons at the
// screen's bottom right, lighter under the mouse, a left click opening the address in the browser, shown when the
// platform allows web links (the play mode). The menu background under the icons is kept and put back before each
// draw, so the blends never pile up.

#include "window/main_menu_links.h"
#include "graphics/graphics_display.h"
#include "graphics/screen_pixels.h"
#include "input/input_data.h"
#include "platform/platform_shell.h"
#include "window/window_data.h"

enum main_menu_link_blend {
    ALPHA_OPAQUE = 255,
    HOVER_LIGHTEN_DIVISOR = 4,
    RED_MAX = 0x1F,
    GREEN_565_MAX = 0x3F,
    BLUE_MAX = 0x1F,
    RGB565_RED_SHIFT = 11,
    RGB565_GREEN_SHIFT = 5,
};

main_menu_link_place main_menu_links[MAIN_MENU_LINKS] = {
    { "https://discord.gg/HS4njmBvpb", 100, 50 },
    { "https://github.com/dalerank/tutankhamun", 150, 50 },
    { "https://dalerank.github.io/en/", 50, 50 },
    { "https://dalerank.github.io/Akhenaten/", 200, 50 },
};

static uint16_t kept_background[MAIN_MENU_LINKS][MAIN_MENU_LINK_ICON_PIXELS];
static int32_t background_kept;

typedef struct rgb565_channels {
    int32_t red;
    int32_t green;
    int32_t blue;
} rgb565_channels;

static int32_t links_shown(void)
{
    return platform_web_links_allowed();
}

static int32_t link_left(int32_t link)
{
    return g_screen_width - main_menu_links[link].right;
}

static int32_t link_top(int32_t link)
{
    return g_screen_height - main_menu_links[link].bottom;
}

static int32_t on_screen(int32_t x, int32_t y)
{
    return x >= 0 && x < g_screen_width && y >= 0 && y < g_screen_height;
}

static int32_t link_under_mouse(int32_t link)
{
    int32_t left = link_left(link);
    int32_t top = link_top(link);
    return mouse_x >= left && mouse_x < left + MAIN_MENU_LINK_ICON_SIZE && mouse_y >= top && mouse_y < top + MAIN_MENU_LINK_ICON_SIZE;
}

static int32_t screen_is_565(void)
{
    return display_mode == DISPLAY_MODE_RGB565;
}

static rgb565_channels channels_of_screen_pixel(uint16_t pixel)
{
    rgb565_channels channels;
    if (screen_is_565()) {
        channels.red = (pixel & PIXEL_565_RED_MASK) >> RGB565_RED_SHIFT;
        channels.green = (pixel & PIXEL_565_GREEN_MASK) >> RGB565_GREEN_SHIFT;
    } else {
        channels.red = (pixel & PIXEL_555_RED_MASK) >> PIXEL_555_RED_SHIFT;
        channels.green = ((pixel & PIXEL_555_GREEN_MASK) >> PIXEL_555_GREEN_SHIFT) << 1;
    }
    channels.blue = pixel & PIXEL_BLUE_MASK;
    return channels;
}

static uint16_t screen_pixel_of_channels(rgb565_channels channels)
{
    if (screen_is_565()) {
        return (uint16_t) ((channels.red << RGB565_RED_SHIFT) | (channels.green << RGB565_GREEN_SHIFT) | channels.blue);
    }
    return (uint16_t) ((channels.red << PIXEL_555_RED_SHIFT) | ((channels.green >> 1) << PIXEL_555_GREEN_SHIFT) | channels.blue);
}

static rgb565_channels icon_channels(uint16_t color, int32_t hovered)
{
    rgb565_channels channels = { (color & PIXEL_565_RED_MASK) >> RGB565_RED_SHIFT, (color & PIXEL_565_GREEN_MASK) >> RGB565_GREEN_SHIFT, color & PIXEL_BLUE_MASK };
    if (hovered) {
        channels.red += (RED_MAX - channels.red) / HOVER_LIGHTEN_DIVISOR;
        channels.green += (GREEN_565_MAX - channels.green) / HOVER_LIGHTEN_DIVISOR;
        channels.blue += (BLUE_MAX - channels.blue) / HOVER_LIGHTEN_DIVISOR;
    }
    return channels;
}

static int32_t blend_channel(int32_t under, int32_t over, int32_t alpha)
{
    return under + (over - under) * alpha / ALPHA_OPAQUE;
}

static uint16_t blend_pixel(uint16_t screen, uint16_t color, int32_t alpha, int32_t hovered)
{
    rgb565_channels under = channels_of_screen_pixel(screen);
    rgb565_channels over = icon_channels(color, hovered);
    rgb565_channels blended = {
        blend_channel(under.red, over.red, alpha),
        blend_channel(under.green, over.green, alpha),
        blend_channel(under.blue, over.blue, alpha),
    };
    return screen_pixel_of_channels(blended);
}

void main_menu_links_keep_background(void)
{
    if (!links_shown()) {
        return;
    }
    for (int32_t link = 0; link < MAIN_MENU_LINKS; link++) {
        for (int32_t row = 0; row < MAIN_MENU_LINK_ICON_SIZE; row++) {
            for (int32_t column = 0; column < MAIN_MENU_LINK_ICON_SIZE; column++) {
                int32_t x = link_left(link) + column;
                int32_t y = link_top(link) + row;
                if (on_screen(x, y)) {
                    kept_background[link][row * MAIN_MENU_LINK_ICON_SIZE + column] = pixel_read(screen_pixel_address(x, y));
                }
            }
        }
    }
    background_kept = 1;
}

static void draw_link(int32_t link)
{
    int32_t hovered = link_under_mouse(link);
    for (int32_t row = 0; row < MAIN_MENU_LINK_ICON_SIZE; row++) {
        for (int32_t column = 0; column < MAIN_MENU_LINK_ICON_SIZE; column++) {
            int32_t x = link_left(link) + column;
            int32_t y = link_top(link) + row;
            if (!on_screen(x, y)) {
                continue;
            }
            int32_t pixel = row * MAIN_MENU_LINK_ICON_SIZE + column;
            uint16_t background = kept_background[link][pixel];
            uint16_t drawn = blend_pixel(background, main_menu_link_icon_colors[link][pixel], main_menu_link_icon_alpha[link][pixel], hovered);
            pixel_write(screen_pixel_address(x, y), drawn);
        }
    }
}

void main_menu_links_draw(void)
{
    if (!links_shown() || !background_kept) {
        return;
    }
    for (int32_t link = 0; link < MAIN_MENU_LINKS; link++) {
        draw_link(link);
    }
}

int32_t main_menu_links_handle_mouse(void)
{
    if (!links_shown() || !mouse_left_went_up) {
        return 0;
    }
    for (int32_t link = 0; link < MAIN_MENU_LINKS; link++) {
        if (link_under_mouse(link)) {
            platform_url_open(main_menu_links[link].url);
            return 1;
        }
    }
    return 0;
}
