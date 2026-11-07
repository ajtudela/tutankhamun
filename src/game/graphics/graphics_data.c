// The built-in data of the graphics: fonts, colors, image and draw tables. graphics/graphics_data.h declares them.

#include "figure/action.h"
#include "graphics/graphics_data.h"
#include "widget/city_view_rotate.h"
#include "window/main_menu.h"

uint8_t arrow_repeat_pattern[40] = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 0 };

const char str_dd_enable_error[28] = "ERR:DD Enable error level:";

const char str_dd_256_cols[44] = "OK :DD 256 cols switching to full screen.";

const char str_dd_fill_failed[32] = "ERR:DD Clearing Fill failed no:";

int32_t g_skip_refresh = 1;

uint8_t dd_mode_retry_flag = 1;

int32_t dd_good_fullscreen = 1;

char bmp_signature[156] = "BM\0\0"
                                  "WinSJBClass\0"
                                  "Debug and status report on \0"
                                  "OK :Exited Normally  \0\0\0"
                                  "OK :Paused.  \0\0\0"
                                  "ERR:Can't restore front buffer.  \0\0\0"
                                  "ERR:Can't restore back buffer.  ";

const char str_gfx_not_enabled[32] = "ERR:GFX DirectDraw not enabled.";

const char str_gfx_begin_failed[24] = "ERR:gfxBegin failed.";

char dialog_loc_name[12] = "dialog.loc";

char g_system_bmp[12] = "system.bmp";

char g_ext_sg3[8] = ".sg3";

char data_path_prefix[6] = "data\\";

char g_ext_555_dot[8] = ".555";

screen_fade fade_object;

uint32_t system_palette[763];

int32_t dash_step;

int32_t dash_color;

int32_t dash_base_color;


int32_t text_draw_shadow;

int32_t g_shadow_c;

uint16_t g_draw_color;

uint8_t font_draw_scratch_end[16];

uint8_t g_clip_y;

uint8_t g_clip_x;

int32_t g_clip_top;

int32_t g_dst_skip;

int32_t g_src_skip;

int32_t g_visible_w;

int32_t g_clip_y1;

int32_t g_clip_y0;

int32_t g_clip_x1;

int32_t g_clip_x0;

int32_t image_draw_last_y;

int32_t image_draw_last_x;

int32_t image_draw_last_id;

uint8_t g_img_type;

intptr_t g_data_ptr;

int16_t g_img_height;

int16_t g_img_width;

int32_t g_visible_h;

int32_t g_draw_y;

int32_t g_draw_x;

int32_t g_max_line_width;

int32_t g_cursor_x_offset;

int32_t g_draw_y_accum;

int32_t g_hash_pos;

int32_t g_hash_mode;

int32_t g_char_index;

int32_t g_cursor_seen;

int32_t g_cursor_width;

int32_t text_cursor_y;

uint8_t g_multiline_flag;

int32_t capture_cursor_position[20];

char string_number_buffer[256];

char tmp_line_buffer[2204];

int32_t dash_blink_frame;

int32_t g_screen_pitch;

int32_t screen_height;

int32_t g_screen_height;

int32_t g_screen_width;

uint8_t g_force_refresh;

uint8_t window_state_end[9];

int32_t dd_enabled;

int32_t window_pos_y;

int32_t window_pos_x;

platform_rect window_screen_rect;

int32_t display_pixel_format;

platform_surface *display_back_surface;

int32_t g_refresh_accum_ms;

char *spare_screen;

int32_t dd_good_display_mode;

int32_t ui_layout_column_widths[10];

int32_t ui_layout_marked_key_x;

int32_t ui_layout_marked_key_y;

int32_t ui_layout_marked_x;

int32_t ui_layout_marked_y;

int32_t ui_layout_marked_width;

int32_t ui_layout_marked_height;

ui_layout_window_table ui_layout_lists;

int32_t ui_layout_edit_mode;

int16_t *layout_focus_button;

int32_t ui_layout_label_width;

int32_t g_drawn_overlay;

int32_t video_smk_playing;

int32_t frame_time_delta;

debug_frame_state debug_frame;
