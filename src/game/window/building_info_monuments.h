// The monument panels of the building info window (building_info_monuments.c): their draws, the text tokens, the
// pyramid style rules and the progress reports the window reads.

#ifndef WINDOW_BUILDING_INFO_MONUMENTS_H
#define WINDOW_BUILDING_INFO_MONUMENTS_H

#include "building/monument_progress_types.h"

#include <stdint.h>

typedef struct {
    int32_t second;
    int32_t first;
} monument_text_resources;

enum pyramid_style_rule_index {
    PYRAMID_STYLE_RULE_OTHER,
    PYRAMID_STYLE_RULE_BENT,
    PYRAMID_STYLE_RULE_MUDBRICK,
    PYRAMID_STYLE_RULE_STEPPED,
    PYRAMID_STYLE_RULE_PYRAMID,
    PYRAMID_STYLE_RULE_MASTABA,
    PYRAMID_STYLE_RULE_COUNT,
};

typedef struct {
    int32_t help_message;
    int32_t main_material;
    int32_t casing_material;
    int32_t stonemasons;
    int32_t bricklayers;
} pyramid_style_rule;

enum {
    MONUMENT_TEXTS_COUNTED = 0x100,
};

extern pyramid_style_rule pyramid_style_rules[PYRAMID_STYLE_RULE_COUNT];
extern library_info_report bi_library_report;
extern uint16_t building_info_monument_texts_drawn[MONUMENT_TEXTS_COUNTED];

void window_building_draw_pyramid(void);
void window_building_draw_sphinx(void);
void window_building_draw_obelisk(void);
void window_building_draw_sun_temple(void);
void window_building_draw_great_library(void);
void window_building_draw_abu_simbel(void);
void window_building_draw_caesareum(void);
void window_building_draw_lighthouse(void);
void window_building_draw_royal_tomb(void);
void window_building_draw_mausoleum(void);
const char *window_building_monument_name(int32_t building_id);
const char *window_building_monument_resource_name(int32_t resource);

#endif
