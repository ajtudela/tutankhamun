// The interface layout: column widths, window keys, entry lookup, the layout editor's mark, the layout file,
// and the texts, labels, borders and inner panels drawn at a layout position.

#ifndef GRAPHICS_UI_LAYOUT_H
#define GRAPHICS_UI_LAYOUT_H

#include "graphics/ui_layout_types.h"
#include <stdint.h>

extern int32_t ui_layout_column_groups[UI_LAYOUT_COLUMNS];

int32_t ui_layout_window_key(int32_t window_id);
ui_layout_entry *ui_layout_find(int32_t kind, int32_t key, int32_t x, int32_t y);
void ui_layout_highlight(int32_t kind, int32_t x, int32_t y, int32_t width, int32_t height, int32_t base_x, int32_t base_y, int32_t offset_x, int32_t offset_y);
void ui_layout_refresh(void);
int32_t ui_layout_group_max_width(int32_t group);
void ui_layout_init_column_widths(void);
int32_t ui_layout_init_label_width(void);
void dialog_loc_read_list(void *file, int32_t count, ui_layout_entry **list);
int32_t dialog_loc_load(void);
void ui_layout_init(void);
void lang_text_draw_ui(int32_t group, int32_t index, int32_t x, int32_t y, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y);
void lang_text_draw_centered_ui(
    int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y);
void lang_text_draw_multiline_ui(
    int32_t group, int32_t index, int32_t x, int32_t y, int32_t box_width, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y);
void text_draw_number_ui(
    int32_t value, int32_t prefix, const char *postfix, int32_t x, int32_t y, int32_t font, int32_t color, int32_t offset_x, int32_t offset_y);
void button_border_draw_ui(int32_t x, int32_t y, int32_t width, int32_t height, int32_t has_focus, int32_t offset_x, int32_t offset_y);
void label_draw_animated_ui(int32_t id, int32_t x, int32_t y, int32_t width_blocks, int32_t force, int32_t type, int32_t offset_x, int32_t offset_y);
void inner_panel_draw_ui(int32_t x, int32_t y, int32_t width_blocks, int32_t height_blocks, int32_t offset_x, int32_t offset_y);
int32_t ui_layout_label_width_get(void);
int32_t ui_layout_label_columns_get(void);

#endif
