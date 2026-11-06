// Crime waves from a scenario event or the "jail break" cheat: each criminal leaves the inhabited house with the most
// crime whose own criminal is not out already, as a tomb robber where the mission allows them.
//
// After a criminal is sent, the crime that is reset is not that of its house but that of the building one past the last
// house looked at.

#include "city/crime.h"

#include "building/building.h"
#include "building/building_data.h"
#include "building/building_maintenance.h"
#include "building/house_evolve.h"
#include "city/city_internal.h"
#include "figure/figure.h"
#include "figure/figure_type.h"

static int32_t is_own_criminal_out(const figure *f, int32_t house_id)
{
    return f->state != FIGURE_STATE_NONE && (f->type == FIGURE_CRIMINAL || f->type == FIGURE_TOMB_ROBBER) && f->building_id == house_id;
}

void city_crime_wave(int32_t count)
{
    int32_t highest_id = building_highest_id;
    for (int32_t wave = 0; wave < count; wave++) {
        int32_t id = 1;
        int32_t max_crime = 0;
        int32_t criminal_house_id = 0;
        for (; id <= highest_id; id++) {
            building *house = building_get(id);
            if (house->state != BUILDING_STATE_IN_USE || !house->house_size || house->house_population == 0) {
                continue;
            }
            int32_t walker_id = house->figure_id;
            if (walker_id > 0) {
                if (is_own_criminal_out(figure_get(walker_id), id)) {
                    continue;
                }
                house->figure_id = 0;
            }
            if (house->crime_risk > max_crime) {
                max_crime = house->crime_risk;
                criminal_house_id = id;
            }
        }
        if (criminal_house_id > 0) {
            figure_generate_criminal(criminal_house_id, figure_criminal_tomb_robbers_possible());
            highest_id = building_highest_id;
            building *building_after_last_house = building_get(id);
            building_after_last_house->crime_risk = house_models[building_after_last_house->subtype].crime_base;
        }
    }
}
