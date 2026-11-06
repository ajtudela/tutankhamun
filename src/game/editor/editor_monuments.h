// The monuments, gods and trade prices windows (editor_monuments.c), the monument checks the editor and the city use,
// the int16 numeric input, and the burial provision tables (editor_monument_data.c).

#ifndef EDITOR_EDITOR_MONUMENTS_H
#define EDITOR_EDITOR_MONUMENTS_H

#include "building/monument.h"
#include "building/monument_tables.h"
#include "window/input_dialogs.h"
#include <stdint.h>

enum burial_provision_rows {
    BURIAL_PROVISION_ROWS = 15,
};

extern int32_t burial_provision_resources[BURIAL_PROVISION_ROWS];
extern int32_t monument_burial_provisions[MONUMENT_DEFINITIONS];

void window_editor_gods_draw(void);
void window_editor_monuments_draw(void);
void window_editor_trade_prices_draw(void);
void editor_open_monuments(void);
void editor_monument_select(void);
void editor_monument_animation_set(void);
void editor_monument_goods_amount(void);
void editor_gods_toggle_temple_complex(void);
void editor_gods_next_status(void);
void editor_trade_price_button(void);
void editor_trade_prices_reset(void);
int32_t monument_def_goods_available(int32_t id);
int32_t monument_def_allowed(int32_t era, const monument_def *def);
int editor_monument_ids_compare(const void *a, const void *b);
void numeric_input_show_int16(int32_t x, int32_t y, int16_t *target, input_dialog_callback on_ok);

#endif
