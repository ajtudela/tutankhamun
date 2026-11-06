// The scenario event manager: once a day it processes every event not only started by another one, re-arms fired
// events (recurring ones at once, favour-triggered ones when their countdown runs out), flags a due earthquake and
// processes the invasions. Also the event list helpers: what an event type uses, the resources a request or gift may
// ask for, the next active request or invasion, a free record for a new event and the drawing of randomised values.
//
// How the random values and the invasion search behave:
// - a value with a range is drawn modulo max - min, so max itself is never drawn;
// - the first random number of an event's reset only advances the seeds;
// - without an invasion the invasion search may hand out the cheats' record.

#include "core/static_init.h"
#include "figure/figure.h"
#include "figure/figure_type.h"
#include "figure/formation.h"
#include "game/game_data.h"
#include "platform/status_log.h"
#include "scenario/scenario_data.h"
#include "scenario/scenario_internal.h"
#include "scenario/scenario_rules.h"

#include <stdbool.h>

enum event_reset_margin {
    EVENT_MONTHS_MARGIN_MIN = 4,
    EVENT_MONTHS_MARGIN_CHOICES = 5,
};

scenario_event *scenario_event_get(int32_t id)
{
    if (id >= 0 && id < scenario_events.count) {
        return &scenario_events.events[id];
    }
    return NULL;
}

bool scenario_event_type_uses_city(scenario_event *e)
{
    return event_type_properties[e->type].uses_city != 0;
}

bool scenario_event_type_is_invasion(scenario_event *e)
{
    return event_type_properties[e->type].is_invasion != 0;
}

int32_t scenario_event_uses_trade_city(scenario_event *e)
{
    if (!scenario_event_type_uses_city(e)) {
        return 0;
    }
    return scenario_event_type_is_invasion(e) == 0;
}

bool scenario_event_type_uses_resource(scenario_event *e)
{
    return event_type_properties[e->type].uses_resource != 0;
}

bool scenario_event_is_chain_target(scenario_event *e, int32_t id)
{
    scenario_event *target = scenario_event_get(id);
    if (!target) {
        return 0;
    }
    if (!(target->flags & EVENT_FLAG_CHAIN_ONLY)) {
        return 0;
    }
    return id != e->id;
}

static int32_t value_range_size(const scenario_value *v)
{
    return v->max - v->min + 1;
}

static int32_t value_fixed_choices(const scenario_value *v)
{
    if (v->min < 0) {
        return 1;
    }
    return (v->max >= 0) + 2;
}

int32_t scenario_value_choice_count(scenario_value *v)
{
    if (v->fixed >= 0) {
        return value_fixed_choices(v);
    }
    if (v->min >= 0 && v->max >= 0) {
        return value_range_size(v);
    }
    return 0;
}

void scenario_value_randomize(scenario_value *v)
{
    if (v->fixed >= 0) {
        if (v->min < 0) {
            v->value = v->fixed;
            return;
        }
    } else if (v->min >= 0 && v->max >= 0 && value_range_size(v) == 1) {
        v->value = v->fixed;
        return;
    }
    random_generate_next();
    int32_t r = random_generator_state.random1_15bit & 0xFFFF;
    if (v->fixed < 0) {
        v->value = (int16_t) ((r % (v->max - v->min)) + v->min);
        return;
    }
    switch (r % value_fixed_choices(v)) {
        case 0:
            v->value = v->fixed;
            break;
        case 1:
            v->value = v->min;
            break;
        default:
            v->value = v->max;
            break;
    }
}

static uint32_t random_advance_seeds(void)
{
    random_pool[random_pool_index++] = random_generator_state.random1_7bit;
    if (random_pool_index >= RANDOM_POOL_SIZE) {
        random_pool_index = 0;
    }
    uint32_t iv1 = random_generator_state.iv1;
    uint32_t iv2 = random_generator_state.iv2;
    for (int32_t i = 0; i < 31; i++) {
        uint32_t feedback1 = ((iv1 >> 4) & 1) ^ (iv1 & 1);
        uint32_t feedback2 = ((iv2 >> 4) & 1) ^ (iv2 & 1);
        iv1 >>= 1;
        iv2 >>= 1;
        if (feedback1) {
            iv1 |= 0x40000000;
        }
        if (feedback2) {
            iv2 |= 0x40000000;
        }
    }
    random_generator_state.iv1 = iv1;
    random_generator_state.random1_7bit = iv1 & 0x7F;
    random_generator_state.iv2 = iv2;
    return iv1;
}

void scenario_event_reset(scenario_event *e)
{
    e->flags &= ~EVENT_FLAG_FIRED;
    scenario_value_randomize(&e->item);
    scenario_value_randomize(&e->amount);
    scenario_value_randomize(&e->year);
    scenario_value_randomize(&e->route);
    scenario_value_randomize(&e->invasion);
    e->invasion_warnings_shown = 0;
    e->state = 0;
    e->months_left = (int16_t) (e->month + e->year.value);
    e->parent_id = -1;
    e->parent_phrase = -1;
    e->is_active = 0;
    e->can_comply_shown = 0;
    e->parent_city = 0;
    if (e->type == EVENT_TYPE_EARTHQUAKE) {
        for (int32_t i = 0; i < SCENARIO_EARTHQUAKE_FRONTS; i++) {
            e->data.earthquake.locations[2 * i] = scenario_data.earthquake_x;
            e->data.earthquake.locations[(2 * i) + 1] = scenario_data.earthquake_y;
        }
    }
    uint32_t iv1 = random_advance_seeds();
    e->months_margin = (uint8_t) (((iv1 & 0x7FFF) % EVENT_MONTHS_MARGIN_CHOICES) + EVENT_MONTHS_MARGIN_MIN);
    random_generate_next();
    e->coin = (uint8_t) (random_generator_state.random1_15bit & 1);
}

static int32_t invasion_has_formation(int32_t event_id)
{
    for (formation *m = formation_get(1); m < formation_get(MAX_FORMATIONS); m++) {
        if (m->in_use == 1 && m->is_legion == 0 && m->herd_kind == 0 && m->invasion_event_id == event_id) {
            return 1;
        }
    }
    return 0;
}

static int32_t invasion_has_transport_ship(int32_t event_id)
{
    for (int32_t i = 1; i < MAX_FIGURES; i++) {
        const figure *f = &city_figures.records[i];
        if (f->state != FIGURE_STATE_NONE && f->type == FIGURE_ENEMY_TRANSPORT_SHIP && f->invasion_event_id == event_id) {
            return 1;
        }
    }
    return 0;
}

static int32_t event_is_reusable(const scenario_event *e)
{
    return (e->flags & EVENT_FLAG_CLONE) && (e->flags & EVENT_FLAG_FIRED) && !e->is_active;
}

scenario_event *scenario_event_add(scenario_events_t *self, bool add_only)
{
    if (add_only && self->count >= SCENARIO_EVENTS_ADD_ONLY_LIMIT) {
        return NULL;
    }
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (!event_is_reusable(e) || scenario_event_is_due(e, 1)) {
            continue;
        }
        if (e->type != EVENT_TYPE_INVASION) {
            return e;
        }
        if (!invasion_has_formation(i) && !invasion_has_transport_ship(i)) {
            return e;
        }
    }
    if (self->count < SCENARIO_EVENTS_LIMIT) {
        self->count++;
        scenario_event *e = &self->events[self->count - 1];
        scenario_event_set_defaults(e);
        e->id = (int16_t) (self->count - 1);
        return e;
    }
    if (!add_only) {
        log_info_text(event_add_error_text, 0);
    }
    return NULL;
}

scenario_event *scenario_request_next_active(scenario_events_t *self, scenario_event *prev)
{
    for (int32_t i = prev ? (prev->id + 1) : 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (e->type == EVENT_TYPE_REQUEST && e->is_active) {
            e->id = (int16_t) i;
            return e;
        }
    }
    return NULL;
}

scenario_event *scenario_invasion_next(scenario_events_t *self, scenario_event *prev, bool make_new)
{
    int32_t start = prev ? (prev->id + 1) : 0;
    self->invasion_search_flag = 1;
    int32_t i = start;
    for (; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (e->type == EVENT_TYPE_INVASION) {
            e->id = (int16_t) i;
            scenario_current_event_index = i;
            return e;
        }
    }
    if (make_new && start < SCENARIO_EVENTS_LIMIT) {
        self->events[i].id = (int16_t) i;
        scenario_current_event_index = i;
        return &self->events[SCENARIO_CHEAT_INVASION_EVENT];
    }
    scenario_current_event_index--;
    return NULL;
}

void scenario_events_rearm(scenario_events_t *self)
{
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        uint32_t flags = e->flags;
        if (!(flags & EVENT_FLAG_FIRED) || e->is_active) {
            continue;
        }
        if (flags & EVENT_FLAG_RECURRING) {
            e->flags = flags & ~EVENT_FLAG_FIRED;
            scenario_event_reset(e);
            e->year.value = (int16_t) (e->year.value + (int16_t) (game_time_year - scenario_data.start.word));
        } else if (flags & EVENT_FLAG_BY_FAVOUR) {
            if (--e->month == 0) {
                e->flags &= ~EVENT_FLAG_FIRED;
                scenario_event_reset(e);
            }
        }
    }
}

void scenario_events_process_all(scenario_events_t *self)
{
    self->reserved_1 = 0;
    self->earthquake_active = 0;
    self->invasion_search_flag = 0;
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (!(e->flags & EVENT_FLAG_CHAIN_ONLY)) {
            scenario_current_event_index = i;
            scenario_event_process(e);
        }
    }
    scenario_current_event_index = -1;
    scenario_events_rearm(self);
}

void scenario_earthquake_process(scenario_events_t *self)
{
    if (editor_active == 1 || scenario_data.earthquake_x == -1 || scenario_data.earthquake_y == -1) {
        return;
    }
    for (int32_t i = 0; i < self->count; i++) {
        scenario_event *e = &self->events[i];
        if (e->type == EVENT_TYPE_EARTHQUAKE && scenario_earthquake_event_process(e)) {
            self->earthquake_active = 1;
            return;
        }
    }
}

void scenario_invasions_process(void)
{
    if (editor_active == 1) {
        return;
    }
    for (scenario_event *e = scenario_invasion_next(&scenario_events, NULL, 0); e; e = scenario_invasion_next(&scenario_events, e, 0)) {
        scenario_event_process_invasion(e);
    }
}

bool scenario_event_processed_later(scenario_events_t *self, int32_t id)
{
    if (scenario_current_event_index < 0) {
        return 0;
    }
    if (id < scenario_current_event_index) {
        return 0;
    }
    if (id < 0 || id >= self->count) {
        return 0;
    }
    if (self->events[id].type == EVENT_TYPE_INVASION) {
        return self->invasion_search_flag != 0;
    }
    return self->invasion_search_flag == 0;
}

scenario_event *scenario_invasion_event_at(scenario_events_t *self, int32_t id)
{
    if (id == SCENARIO_CHEAT_INVASION_EVENT) {
        return &self->events[SCENARIO_CHEAT_INVASION_EVENT];
    }
    if (id >= 0 && id < self->count) {
        scenario_event *e = &self->events[id];
        return (e->type == EVENT_TYPE_INVASION) ? e : NULL;
    }
    return NULL;
}

int32_t scenario_event_type_resource_min(scenario_event *e)
{
    return event_type_properties[e->type].item_min;
}

int32_t scenario_event_type_resource_max(scenario_event *e)
{
    return event_type_properties[e->type].item_max;
}

static bool request_resource_is_festival_supply(int32_t resource)
{
    if (resource >= RESOURCE_MIN_FOOD && resource < RESOURCE_MAX_FOOD) {
        return 1;
    }
    switch (resource) {
        case RESOURCE_BEER:
        case RESOURCE_LINEN:
        case RESOURCE_LUXURY_GOODS:
        case RESOURCE_DEBEN:
            return 1;
    }
    return 0;
}

static bool request_resource_is_building_material(int32_t resource)
{
    switch (resource) {
        case RESOURCE_BRICKS:
        case RESOURCE_TIMBER:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_SANDSTONE:
        case RESOURCE_DEBEN:
            return 1;
    }
    return 0;
}

bool scenario_request_resource_allowed(scenario_event *e, int32_t resource)
{
    switch (e->subtype) {
        case 0:
        case 6:
            return resource != RESOURCE_TROOPS && resource != RESOURCE_GOLD && resource != RESOURCE_UNUSED;
        case 1:
        case 2:
            return resource == RESOURCE_TROOPS;
        case 3:
            return request_resource_is_festival_supply(resource);
        case 4:
            return request_resource_is_building_material(resource);
        case 5:
            return resource >= RESOURCE_MIN_FOOD && resource < RESOURCE_MAX_FOOD;
    }
    return 0;
}

bool scenario_gift_resource_allowed(int32_t resource)
{
    return resource != RESOURCE_TROOPS && resource != RESOURCE_UNUSED;
}

bool scenario_event_resource_in_range(scenario_event *e, int32_t resource)
{
    if (resource < scenario_event_type_resource_min(e) || resource >= scenario_event_type_resource_max(e)) {
        return 0;
    }
    switch (e->type) {
        case EVENT_TYPE_REQUEST:
            return scenario_request_resource_allowed(e, resource);
        case EVENT_TYPE_GIFT_FROM_PHARAOH:
            return scenario_gift_resource_allowed(resource);
        case EVENT_TYPE_DEMAND_INCREASE:
        case EVENT_TYPE_DEMAND_DECREASE:
        case EVENT_TYPE_PRICE_INCREASE:
        case EVENT_TYPE_PRICE_DECREASE:
            return resource != RESOURCE_UNUSED && resource != RESOURCE_GOLD;
    }
    return 1;
}
