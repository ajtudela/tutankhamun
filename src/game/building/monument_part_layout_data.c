// The built-in part layouts of the scripted monuments: per part its tile offset from the origin, its size and its
// kind. config/monument_part_layouts.json can override them.

#include "building/monument_object.h"

static monument_part_init library_parts[LIBRARY_PARTS] = {
    { -3, -4, 1, 23 },
    { 2, -4, 1, 24 },
    { -3, -3, 6, 0 },
    { -2, 3, 4, 1 },
    { -2, -7, 4, 2 },
    { -3, 3, 1, 7 },
    { 2, 3, 1, 8 },
    { -4, -2, 1, 17 },
    { 3, -2, 1, 18 },
    { -5, -1, 2, 13 },
    { 3, -1, 2, 14 },
    { -4, 1, 1, 10 },
    { 3, 1, 1, 11 },
    { -5, 4, 3, 3 },
    { 2, 4, 3, 4 },
    { -5, -7, 3, 21 },
    { 2, -7, 3, 22 },
    { 3, 2, 2, 6 },
    { -5, 2, 2, 5 },
    { -5, -4, 2, 19 },
    { 3, -4, 2, 20 },
    { -5, 1, 1, 9 },
    { 4, 1, 1, 12 },
    { -5, -2, 1, 15 },
    { 4, -2, 1, 16 },
};

static monument_part_init caesareum_parts[CAESAREUM_PARTS] = {
    { -7, -7, 5, 0 },
    { -7, -2, 5, 1 },
    { -7, 3, 5, 2 },
    { -2, -7, 5, 3 },
    { -2, -2, 5, 4 },
    { -2, 3, 5, 5 },
    { 3, -7, 5, 6 },
    { 3, -2, 5, 7 },
    { 3, 3, 5, 8 },
};

static monument_part_init lighthouse_parts[LIGHTHOUSE_PARTS] = {
    { 0, 0, 6, 0 },
    { 2, 6, 1, 1 },
};

static monument_part_init abu_simbel_parts[ABU_SIMBEL_PARTS] = {
    { 1, 7, 3, 5 },
    { 1, -11, 3, 6 },
    { -5, 4, 3, 0 },
    { -5, 1, 3, 0 },
    { -5, -2, 3, 0 },
    { -5, -5, 3, 0 },
    { -5, -8, 3, 0 },
    { -2, 7, 3, 1 },
    { -2, -11, 3, 2 },
    { -5, 7, 3, 3 },
    { -5, -11, 3, 4 },
    { -2, -2, 3, 7 },
    { 1, -2, 3, 8 },
    { 4, -2, 3, 9 },
    { -2, 1, 6, 10 },
    { -2, -8, 6, 11 },
};

const monument_part_layout monument_part_layouts[MONUMENT_LAYOUT_COUNT] = {
    [MONUMENT_LAYOUT_LIBRARY] = { library_parts, LIBRARY_PARTS },
    [MONUMENT_LAYOUT_CAESAREUM] = { caesareum_parts, CAESAREUM_PARTS },
    [MONUMENT_LAYOUT_LIGHTHOUSE] = { lighthouse_parts, LIGHTHOUSE_PARTS },
    [MONUMENT_LAYOUT_ABU_SIMBEL] = { abu_simbel_parts, ABU_SIMBEL_PARTS },
};
