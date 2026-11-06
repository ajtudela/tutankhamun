// The built-in goods the empire editor's goods slot list offers for a city's sold goods: the foods and the raw
// materials. Overridden by config/empire_editor_goods_slot_goods.json.

#include "city/resource_type.h"
#include "editor/empire_editor_object_panel.h"

int32_t empire_editor_goods_slot_offered[RESOURCE_MAX] = {
    [RESOURCE_GRAIN] = 1,
    [RESOURCE_MEAT] = 1,
    [RESOURCE_LETTUCE] = 1,
    [RESOURCE_CHICKPEAS] = 1,
    [RESOURCE_POMEGRANATES] = 1,
    [RESOURCE_FIGS] = 1,
    [RESOURCE_FISH] = 1,
    [RESOURCE_GAMEMEAT] = 1,
    [RESOURCE_CLAY] = 1,
    [RESOURCE_BARLEY] = 1,
    [RESOURCE_FLAX] = 1,
    [RESOURCE_GEMS] = 1,
    [RESOURCE_TIMBER] = 1,
    [RESOURCE_REEDS] = 1,
    [RESOURCE_STONE] = 1,
    [RESOURCE_LIMESTONE] = 1,
    [RESOURCE_GRANITE] = 1,
    [RESOURCE_COPPER] = 1,
    [RESOURCE_SANDSTONE] = 1,
    [RESOURCE_HENNA] = 1,
};
