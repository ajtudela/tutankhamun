// The gameplay config table of the ships' escapes (core/config.h): ship_escape_moves.json, per direction from the
// closest enemy the three points an evading ship tries to sail to, for the player's and the enemy's ships alike.

#include "core/config.h"
#include "figure/enemy_ships.h"

static const config_field ship_escape_move_fields[] = {
    { "dx1", &ship_escape_moves[0].dx1, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dy1", &ship_escape_moves[0].dy1, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dx2", &ship_escape_moves[0].dx2, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dy2", &ship_escape_moves[0].dy2, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dx3", &ship_escape_moves[0].dx3, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
    { "dy3", &ship_escape_moves[0].dy3, (int32_t) (sizeof(ship_escape_move) / sizeof(int32_t)), 1, INT8_MIN, INT8_MAX, 0 },
};

static const config_storage ship_escape_move_storage[] = {
    { ship_escape_moves, sizeof(ship_escape_moves) },
};

const config_table ship_escape_move_table = {
    "ship_escape_moves.json",
    "direction",
    config_direction_names,
    DIRECTION_COUNT,
    CONFIG_TABLE_COLUMNS(ship_escape_move_fields, ship_escape_move_storage),
};
