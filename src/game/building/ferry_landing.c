// Ferry landings: the two steps of placing a pair with the build tool and the cancel, the ferry queue records
// (created, cleared, rebuilt after a load or an undo) and the ferry paths.

#include "building/ferry_landing.h"
#include "building/building.h"
#include "building/building_type.h"
#include "city/message.h"
#include "core/calc.h"
#include "game/undo_build.h"
#include <stdint.h>
#include <string.h>

void construction_ferry_landing_placed(int32_t building_id)
{
    if (construction_ferry_state == FERRY_PLACEMENT_NONE) {
        construction_ferry_state = FERRY_PLACEMENT_FIRST_PLACED;
        city_warning_show(WARNING_PLACE_OTHER_FERRY_LANDING);
        return;
    }
    int32_t was = construction_ferry_state;
    construction_ferry_state = FERRY_PLACEMENT_NONE;
    if (was == FERRY_PLACEMENT_PAIR_LINKED) {
        game_undo_add_building(construction_ferry_first);
    }
}

void construction_ferry_cancel(int32_t building_id)
{
    construction_ferry_state = FERRY_PLACEMENT_NONE;
    building_get(building_id)->state = BUILDING_STATE_DELETED;
}

int32_t building_ferry_record_create(int32_t building_id)
{
    for (int32_t id = 1; id < FERRY_QUEUES; id++) {
        ferry_queue *record = ferry_queue_get(id);
        if (record->queue.in_use == 0) {
            memset(record, 0, sizeof(ferry_queue));
            record->queue.in_use = 1;
            record->building_id = building_id;
            return id;
        }
    }
    return 0;
}

void ferry_queues_clear_all(void)
{
    memset(ferry_queues, 0, sizeof(ferry_queues));
}

void ferry_paths_clear_all(void)
{
    memset(ferry_path_records, 0, sizeof(ferry_path_records));
}

void building_ferry_records_update(void)
{
    for (int32_t slot = 1; slot < FERRY_QUEUES; slot++) {
        ferry_queue_get(slot)->building_id = 0;
    }
    int32_t last = building_highest_id;
    if (last < 1) {
        return;
    }
    for (int32_t id = 1; id <= last; id++) {
        const building *b = building_get(id);
        if (b->state != BUILDING_STATE_UNUSED && b->type == BUILDING_FERRY_LANDING && b->queue_index != 0) {
            ferry_queue_get(b->queue_index)->queue.in_use = 1;
        }
    }
    for (int32_t id = 1; id <= building_highest_id; id++) {
        building *b = building_get(id);
        if (b->state == BUILDING_STATE_UNUSED || b->type != BUILDING_FERRY_LANDING) {
            continue;
        }
        uint8_t queue_index = b->queue_index;
        if (queue_index == 0 || ferry_queue_get(queue_index)->building_id != 0) {
            b->queue_index = (uint8_t) building_ferry_record_create(id);
        } else {
            ferry_queue_get(queue_index)->building_id = id;
        }
    }
}
