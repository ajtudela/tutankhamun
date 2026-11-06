// The event processors of scenario/scenario_events.c.

#ifndef SCENARIO_SCENARIO_EVENTS_H
#define SCENARIO_SCENARIO_EVENTS_H

#include "scenario/scenario_event_types.h"

#include <stdint.h>

void scenario_event_process_reputation_increase(scenario_event *e);
void scenario_event_process_sea_trade_problem(scenario_event *e);
void scenario_event_process_land_trade_problem(scenario_event *e);
void scenario_event_process_wage_increase(scenario_event *e);
void scenario_event_process_wage_decrease(scenario_event *e);
void scenario_event_process_contaminated_water(scenario_event *e);
void scenario_event_process_clay_pit_flood(scenario_event *e);
void scenario_event_process_locusts(scenario_event *e);
void scenario_event_process_message(scenario_event *e);
void scenario_event_process_flood_quality(scenario_event *e, int32_t better);
void scenario_event_process_demand_increase(scenario_event *e);
void scenario_event_process_demand_decrease(scenario_event *e);
void scenario_event_process_price_increase(scenario_event *e);
void scenario_event_process_price_decrease(scenario_event *e);
void scenario_event_process_city_status_change(scenario_event *e);
void scenario_event_process_gift_from_pharaoh(scenario_event *e);
void scenario_event_process_invasion(scenario_event *e);
void scenario_event_process_request(scenario_event *e);
void scenario_event_process_troops_request(scenario_event *e);
void scenario_event_chain(scenario_event *e, int32_t id, int32_t phrase);
void scenario_event_process(scenario_event *e);
void scenario_invasion_called_off(scenario_event *e);

#endif
