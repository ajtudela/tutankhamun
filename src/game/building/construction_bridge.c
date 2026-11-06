// Bridges on the build tool: once the drag ends, the bridge's tiles from the start tile get road terrain and the
// sprite of each piece (a ship bridge has ramps, a raised middle and pillars), and the water and land routing is
// updated. Bridges shorter than 2 tiles (ship bridges: 5) are dropped.

#include "building/construction_bridge.h"
#include "building/building_data.h"
#include "map/bridge.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "widget/widget_data.h"
#include <stdint.h>

void map_bridge_add(int32_t x, int32_t y, int32_t is_ship_bridge)
{
    int32_t pillar = 0;
    if (bridge_end_grid_offset <= 0) {
        construction_bridge_length = 0;
        return;
    }
    int32_t length = construction_bridge_length;
    if (length < BRIDGE_MIN_LENGTH) {
        construction_bridge_length = 0;
        return;
    }
    if (is_ship_bridge == 1 && length < SHIP_BRIDGE_MIN_LENGTH) {
        construction_bridge_length = 0;
        return;
    }
    int32_t offset = map_grid_offset(x, y);
    int32_t dir = bridge_direction - map_info.orientation;
    bridge_direction = dir;
    if (dir < 0) {
        dir += DIRECTION_COUNT;
        bridge_direction = dir;
    }
    if (is_ship_bridge == 1) {
        switch (length) {
            case 9:
            case 10:
                pillar = 4;
                break;
            case 11:
            case 12:
                pillar = 5;
                break;
            case 13:
            case 14:
                pillar = 6;
                break;
            case 15:
                pillar = 7;
                break;
            default:
                pillar = (length != 16) ? 8 : 7;
                break;
        }
    }
    int32_t step = construction_bridge_step;
    uint8_t *sprite = &map_grids.sprite[offset];
    uint32_t *terrain = &map_grids.terrain[offset];
    for (int32_t i = 0; i < length; i++, sprite += step, terrain += step) {
        *terrain |= TERRAIN_ROAD;
        if (is_ship_bridge == 0) {
            if (i == (length - 1)) {
                if (dir == DIRECTION_BOTTOM) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM;
                } else if (dir == DIRECTION_LEFT) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT;
                } else if (dir == DIRECTION_TOP) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP;
                } else if (dir == DIRECTION_RIGHT) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT;
                }
            } else if (i == 0) {
                if (dir == DIRECTION_BOTTOM) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_TOP;
                } else if (dir == DIRECTION_LEFT) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_RIGHT;
                } else if (dir == DIRECTION_TOP) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_BOTTOM;
                } else if (dir == DIRECTION_RIGHT) {
                    *sprite = BRIDGE_SPRITE_LOW_RAMP_DOWN_LEFT;
                }
            } else if (dir == DIRECTION_BOTTOM || dir == DIRECTION_TOP) {
                *sprite = BRIDGE_SPRITE_LOW_SPAN_ALONG_Y;
            } else if (dir == DIRECTION_LEFT || dir == DIRECTION_RIGHT) {
                *sprite = BRIDGE_SPRITE_LOW_SPAN_ALONG_X;
            }
        } else if ((i == (length - 2)) || (i == 1)) {
            *sprite = BRIDGE_SPRITE_SHIP_SECOND_TILE;
        } else if (i == (length - 1)) {
            if (dir == DIRECTION_BOTTOM) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_BOTTOM;
            } else if (dir == DIRECTION_LEFT) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_LEFT;
            } else if (dir == DIRECTION_TOP) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_TOP;
            } else if (dir == DIRECTION_RIGHT) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_RIGHT;
            }
        } else if (i == 0) {
            if (dir == DIRECTION_BOTTOM) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_TOP;
            } else if (dir == DIRECTION_LEFT) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_RIGHT;
            } else if (dir == DIRECTION_TOP) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_BOTTOM;
            } else if (dir == DIRECTION_RIGHT) {
                *sprite = BRIDGE_SPRITE_SHIP_RAMP_DOWN_LEFT;
            }
        } else if (i == pillar) {
            if (dir == DIRECTION_BOTTOM || dir == DIRECTION_TOP) {
                *sprite = BRIDGE_SPRITE_SHIP_PILLAR_ALONG_Y;
            } else if (dir == DIRECTION_LEFT || dir == DIRECTION_RIGHT) {
                *sprite = BRIDGE_SPRITE_SHIP_PILLAR_ALONG_X;
            }
        } else if (dir == DIRECTION_BOTTOM || dir == DIRECTION_TOP) {
            *sprite = BRIDGE_SPRITE_SHIP_SPAN_ALONG_Y;
        } else if (dir == DIRECTION_LEFT || dir == DIRECTION_RIGHT) {
            *sprite = BRIDGE_SPRITE_SHIP_SPAN_ALONG_X;
        }
    }
    map_routing_update_water();
    map_routing_update_land_citizen();
    map_routing_update_land_noncitizen();
}
