// Built-in pieces of the bridge ghosts; config/bridge_ghost_pieces.json can override them.

#include "widget/city_building_ghost.h"

bridge_ghost_piece_set bridge_ghost_pieces[BRIDGE_GHOST_DIRECTIONS] = {
    [0] = {
        .low_start = 3,
        .low_end = 5,
        .low_span = 4,
        .ship_start = { 11, -3, -50 },
        .ship_end = { 9, 1, -24 },
        .ship_middle = { 13, 0, -38 },
        .ship_span = { 10, 0, -37 },
    },
    [1] = {
        .low_start = 0,
        .low_end = 2,
        .low_span = 1,
        .ship_start = { 6, -1, -12 },
        .ship_end = { 8, 7, -39 },
        .ship_middle = { 12, 7, -38 },
        .ship_span = { 7, 7, -38 },
    },
    [2] = {
        .low_start = 5,
        .low_end = 3,
        .low_span = 4,
        .ship_start = { 9, -30, -12 },
        .ship_end = { 11, -34, -35 },
        .ship_middle = { 13, 0, -38 },
        .ship_span = { 10, 0, -37 },
    },
    [3] = {
        .low_start = 2,
        .low_end = 0,
        .low_span = 1,
        .ship_start = { 8, -23, -53 },
        .ship_end = { 6, -29, -22 },
        .ship_middle = { 12, 7, -38 },
        .ship_span = { 7, 7, -38 },
    },
};
