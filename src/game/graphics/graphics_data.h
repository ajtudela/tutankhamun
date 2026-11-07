// The tables of graphics/graphics_data.c.

#ifndef GRAPHICS_GRAPHICS_DATA_H
#define GRAPHICS_GRAPHICS_DATA_H

#include "graphics/button.h"
#include "graphics/screen_fade.h"
#include "graphics/ui_layout_types.h"
#include "platform/platform_display.h"
#include "platform/platform_rect.h"
#include "widget/debug_info_types.h"

#include <stdint.h>

extern uint8_t arrow_repeat_pattern[40];

extern const char str_dd_enable_error[28];

extern const char str_dd_256_cols[44];

extern const char str_dd_fill_failed[32];

extern int32_t g_skip_refresh;

extern uint8_t dd_mode_retry_flag;

extern int32_t dd_good_fullscreen;

extern char bmp_signature[156];

extern const char str_gfx_not_enabled[32];

extern const char str_gfx_begin_failed[24];

extern char dialog_loc_name[12];

extern char g_system_bmp[12];

extern char g_ext_sg3[8];

extern char data_path_prefix[6];

extern char g_ext_555_dot[8];

extern screen_fade fade_object;

extern uint32_t system_palette[763];

extern int32_t dash_step;

extern int32_t dash_color;

extern int32_t dash_base_color;


extern int32_t text_draw_shadow;

extern int32_t g_shadow_c;

extern uint16_t g_draw_color;

extern uint8_t font_draw_scratch_end[16];

extern uint8_t g_clip_y;

extern uint8_t g_clip_x;

extern int32_t g_clip_top;

extern int32_t g_dst_skip;

extern int32_t g_src_skip;

extern int32_t g_visible_w;

extern int32_t g_clip_y1;

extern int32_t g_clip_y0;

extern int32_t g_clip_x1;

extern int32_t g_clip_x0;

extern int32_t image_draw_last_y;

extern int32_t image_draw_last_x;

extern int32_t image_draw_last_id;

extern uint8_t g_img_type;

extern intptr_t g_data_ptr;

extern int16_t g_img_height;

extern int16_t g_img_width;

extern int32_t g_visible_h;

extern int32_t g_draw_y;

extern int32_t g_draw_x;

extern int32_t g_max_line_width;

extern int32_t g_cursor_x_offset;

extern int32_t g_draw_y_accum;

extern int32_t g_hash_pos;

extern int32_t g_hash_mode;

extern int32_t g_char_index;

extern int32_t g_cursor_seen;

extern int32_t g_cursor_width;

extern int32_t text_cursor_y;

extern uint8_t g_multiline_flag;

extern int32_t capture_cursor_position[20];

extern char string_number_buffer[256];

extern char tmp_line_buffer[2204];

extern int32_t dash_blink_frame;

extern int32_t g_screen_pitch;

extern int32_t screen_height;

extern int32_t g_screen_height;

extern int32_t g_screen_width;

extern uint8_t g_force_refresh;

extern uint8_t window_state_end[9];

extern int32_t dd_enabled;

extern int32_t window_pos_y;

extern int32_t window_pos_x;

extern platform_rect window_screen_rect;

extern int32_t display_pixel_format;

extern platform_surface *display_back_surface;


extern int32_t g_refresh_accum_ms;

#define SPARE_SCREEN_MIN_BYTES 1200000

extern char *spare_screen;

extern int32_t dd_good_display_mode;

extern int32_t ui_layout_column_widths[10];

extern int32_t ui_layout_marked_key_x;

extern int32_t ui_layout_marked_key_y;

extern int32_t ui_layout_marked_x;

extern int32_t ui_layout_marked_y;

extern int32_t ui_layout_marked_width;

extern int32_t ui_layout_marked_height;

extern ui_layout_window_table ui_layout_lists;

extern int32_t ui_layout_edit_mode;

extern int16_t *layout_focus_button;

extern int32_t ui_layout_label_width;

extern int32_t g_drawn_overlay;

extern int32_t video_smk_playing;

extern int32_t frame_time_delta;

extern debug_frame_state debug_frame;

#endif
