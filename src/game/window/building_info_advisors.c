// The advisor buttons of the building info window: which advisors a building's window offers (fixed ones, labor while
// it lacks workers, monuments while they need its material or product, kingdom while a food or goods request waits,
// a house's evolve text advisor), their draw at the bottom of the window, tooltips and clicks.
//
// An evolve text past the table's 80 entries offers the trade advisor.

#include "window/building_info_advisors.h"
#include "building/building.h"
#include "building/building_industry.h"
#include "building/building_type.h"
#include "building/house_evolve.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "city/resource_type.h"
#include "game/game_data.h"
#include "game/game_internal.h"
#include "graphics/arrow_buttons.h"
#include "graphics/graphics_data.h"
#include "graphics/image_buttons.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/building_info_panels.h"
#include "window/window_data.h"
#include <stdint.h>
#include <string.h>

enum {
    ADVISOR_BUTTONS_X = 40,
    ADVISOR_BUTTONS_FROM_BOTTOM = 40,
    ADVISOR_BUTTON_SPACING = 27,
    ADVISOR_BUTTON_TOOLTIP_BASE = 40,
    REQUESTS_LOOKED_AT = 5,
    WORK_CAMP_MONUMENTS = 3,
};

int32_t house_evolve_advisors[EVOLVE_TEXT_COUNT] = {
    [EVOLVE_TEXT_DEVOLVING_DESIRABILITY] = ADVISOR_CHIEF,
    [EVOLVE_TEXT_NO_WELL] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NO_WATER_SUPPLY] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NO_ENTERTAINMENT] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_ENTERTAINMENT_BELOW_10] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_ENTERTAINMENT_BELOW_25] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_ENTERTAINMENT_BELOW_50] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_ENTERTAINMENT_BELOW_80] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_ENTERTAINMENT_HIGH] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_FOOD_TYPES_1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_FOOD_TYPES_2] = ADVISOR_TRADE,
    [EVOLVE_TEXT_FOOD_TYPES_3] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NO_EDUCATION] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_NO_LIBRARY] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_NO_SCHOOL] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_COURTHOUSE] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_POTTERY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_RELIGION_1] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_RELIGION_2] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_RELIGION_3] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_DENTIST] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_HEALTH_1] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_HEALTH_2] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_LINEN] = ADVISOR_TRADE,
    [EVOLVE_TEXT_BEER] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_LEVEL] = ADVISOR_CHIEF,
    [EVOLVE_TEXT_NEXT_NO_WELL] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_NO_WATER_SUPPLY] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT + 1] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT + 2] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT + 3] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT + 4] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT + 5] = ADVISOR_ENTERTAINMENT,
    [EVOLVE_TEXT_NEXT_FOOD_TYPES_1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_FOOD_TYPES_2] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_FOOD_TYPES_3] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_NO_EDUCATION] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_NEXT_NO_LIBRARY] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_NEXT_NO_SCHOOL] = ADVISOR_EDUCATION,
    [EVOLVE_TEXT_NEXT_COURTHOUSE] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_POTTERY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_RELIGION_1] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_NEXT_RELIGION_2] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_NEXT_RELIGION_3] = ADVISOR_RELIGION,
    [EVOLVE_TEXT_NEXT_DENTIST] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_HEALTH_1] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_HEALTH_2_HAS_PHYSICIAN] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_HEALTH_2] = ADVISOR_HEALTH,
    [EVOLVE_TEXT_NEXT_LINEN] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_BEER] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_DESIRABILITY_E] = ADVISOR_CHIEF,
    [EVOLVE_TEXT_NEXT_JEWELRY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_JEWELRY + 1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_SECOND_JEWELRY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_SECOND_JEWELRY + 1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_JEWELRY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_JEWELRY + 1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY + 1] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY + 2] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY + 3] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY + 4] = ADVISOR_TRADE,
    [EVOLVE_TEXT_NEXT_SECOND_JEWELRY + 5] = ADVISOR_TRADE,
};

building_info_advisor_offer building_info_advisor_offers[BUILDING_TYPE_COUNT] = {
    [BUILDING_HOUSE_CRUDE_HUT] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_STURDY_HUT] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_MEAGER_SHANTY] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_COMMON_SHANTY] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_ROUGH_COTTAGE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_ORDINARY_COTTAGE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_MODEST_HOMESTEAD] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_SPACIOUS_HOMESTEAD] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_MODEST_APARTMENT] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_SPACIOUS_APARTMENT] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_COMMON_RESIDENCE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_SPACIOUS_RESIDENCE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_ELEGANT_RESIDENCE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_FANCY_RESIDENCE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_COMMON_MANOR] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_SPACIOUS_MANOR] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_ELEGANT_MANOR] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_STATELY_MANOR] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_MODEST_ESTATE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_HOUSE_PALATIAL_ESTATE] = { .advisors = { ADVISOR_POPULATION }, .burial_provisions = 1, .house_evolve = 1 },
    [BUILDING_BANDSTAND] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_BOOTH] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_SENET_HOUSE] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_PAVILION] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_CONSERVATORY] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_DANCE_SCHOOL] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_JUGGLER_SCHOOL] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_BULLFIGHT_SCHOOL] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
    [BUILDING_PLAZA] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_GARDENS] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_FORT_CHARIOTEERS] = { .advisors = { ADVISOR_MILITARY } },
    [BUILDING_SMALL_STATUE] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_MEDIUM_STATUE] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_LARGE_STATUE] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_FORT_ARCHERS] = { .advisors = { ADVISOR_MILITARY } },
    [BUILDING_FORT_INFANTRY] = { .advisors = { ADVISOR_MILITARY } },
    [BUILDING_APOTHECARY] = { .advisors = { ADVISOR_HEALTH }, .labor = 1 },
    [BUILDING_MORTUARY] = { .advisors = { ADVISOR_HEALTH }, .labor = 1 },
    [BUILDING_DENTIST] = { .advisors = { ADVISOR_HEALTH }, .labor = 1 },
    [BUILDING_SCRIBAL_SCHOOL] = { .advisors = { ADVISOR_EDUCATION }, .labor = 1 },
    [BUILDING_LIBRARY] = { .advisors = { ADVISOR_EDUCATION }, .labor = 1 },
    [BUILDING_POLICE_STATION] = { .advisors = { ADVISOR_CHIEF }, .labor = 1 },
    [BUILDING_TEMPLE_OSIRIS] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_RA] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_PTAH] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_SETH] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_BAST] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_COMPLEX_OSIRIS] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_COMPLEX_RA] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_COMPLEX_PTAH] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_COMPLEX_SETH] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_TEMPLE_COMPLEX_BAST] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_BAZAAR] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_GRANARY] = { .advisors = { ADVISOR_POPULATION }, .labor = 1, .kingdom_request = ADVISOR_REQUEST_FOOD },
    [BUILDING_STORAGE_YARD] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .kingdom_request = ADVISOR_REQUEST_GOODS },
    [BUILDING_SHIPWRIGHT] = { .advisors = { ADVISOR_FLEET }, .labor = 1 },
    [BUILDING_DOCK] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_FISHING_WHARF] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_PERSONAL_MANSION] = { .advisors = { ADVISOR_KINGDOM }, .labor = 1 },
    [BUILDING_FAMILY_MANSION] = { .advisors = { ADVISOR_KINGDOM }, .labor = 1 },
    [BUILDING_DYNASTY_MANSION] = { .advisors = { ADVISOR_KINGDOM }, .labor = 1 },
    [BUILDING_ARCHITECT_POST] = { .advisors = { ADVISOR_CHIEF }, .labor = 1 },
    [BUILDING_LOW_BRIDGE] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_SHIP_BRIDGE] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_TAX_COLLECTOR] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_TAX_COLLECTOR_UP] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_WATER_LIFT] = { .labor = 1 },
    [BUILDING_WELL] = { .advisors = { ADVISOR_HEALTH } },
    [BUILDING_MILITARY_ACADEMY] = { .advisors = { ADVISOR_MILITARY }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_RECRUITER] = { .advisors = { ADVISOR_MILITARY }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_BARLEY_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_FLAX_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_GRAIN_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_LETTUCE_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_POMEGRANATES_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_CHICKPEAS_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_STONE_QUARRY] = { .labor = 1, .burial_provisions = 1, .monument_material = RESOURCE_STONE },
    [BUILDING_LIMESTONE_QUARRY] = { .labor = 1, .burial_provisions = 1, .monument_material = RESOURCE_LIMESTONE },
    [BUILDING_WOOD_CUTTER] = { .labor = 1, .burial_provisions = 1, .monument_material = RESOURCE_TIMBER },
    [BUILDING_CLAY_PIT] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_BREWERY] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_WEAVER] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_WEAPONSMITH] = { .advisors = { ADVISOR_MILITARY, ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_JEWELER] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_POTTER] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_HUNTING_LODGE] = { .advisors = { ADVISOR_POPULATION }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_FERRY_LANDING] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_ROADBLOCK] = { .advisors = { ADVISOR_CHIEF } },
    [BUILDING_SHRINE_OSIRIS] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_RA] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_PTAH] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_SETH] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_BAST] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_GOD_5] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_GOD_6] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_GOD_7] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_GOD_8] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_SHRINE_GOD_9] = { .advisors = { ADVISOR_RELIGION } },
    [BUILDING_GOLD_MINE] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_GEMSTONE_MINE] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_FIREHOUSE] = { .advisors = { ADVISOR_CHIEF }, .labor = 1 },
    [BUILDING_WALL] = { .advisors = { ADVISOR_MILITARY } },
    [BUILDING_GATEHOUSE] = { .advisors = { ADVISOR_MILITARY }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_TOWER] = { .advisors = { ADVISOR_MILITARY }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_CARPENTERS_GUILD] = { .advisors = { ADVISOR_MONUMENTS }, .labor = 1 },
    [BUILDING_BRICKLAYERS_GUILD] = { .advisors = { ADVISOR_MONUMENTS }, .labor = 1 },
    [BUILDING_STONEMASONS_GUILD] = { .advisors = { ADVISOR_MONUMENTS }, .labor = 1 },
    [BUILDING_WATER_SUPPLY] = { .advisors = { ADVISOR_HEALTH }, .labor = 1 },
    [BUILDING_TRANSPORT_WHARF] = { .advisors = { ADVISOR_FLEET }, .labor = 1 },
    [BUILDING_WARSHIP_WHARF] = { .advisors = { ADVISOR_FLEET }, .labor = 1 },
    [BUILDING_PYRAMID] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_COURTHOUSE] = { .advisors = { ADVISOR_CHIEF }, .labor = 1 },
    [BUILDING_VILLAGE_PALACE_UP] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_TOWN_PALACE_UP] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_CITY_PALACE] = { .advisors = { ADVISOR_FINANCIAL, ADVISOR_RATINGS }, .labor = 1 },
    [BUILDING_BAZAAR_UP] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_DOCK_UP] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_CATTLE_RANCH] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_REED_GATHERER] = { .advisors = { ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_FIG_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_WORK_CAMP] = { .labor = 1, .monument_workers = 1 },
    [BUILDING_GATEHOUSE_UP] = { .advisors = { ADVISOR_MILITARY }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_PAPYRUS_MAKER] = { .advisors = { ADVISOR_TRADE, ADVISOR_EDUCATION }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_BRICKWORKS] = { .labor = 1, .monument_material = RESOURCE_BRICKS },
    [BUILDING_CHARIOT_MAKER] = { .advisors = { ADVISOR_MILITARY, ADVISOR_TRADE }, .labor = 1, .burial_provisions = 1 },
    [BUILDING_PHYSICIAN] = { .advisors = { ADVISOR_HEALTH }, .labor = 1 },
    [BUILDING_SMALL_OBELISK] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_LARGE_OBELISK] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_FESTIVAL_SQUARE] = { .advisors = { ADVISOR_RELIGION }, .labor = 1 },
    [BUILDING_SPHINX] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_SUN_TEMPLE] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_GRANITE_QUARRY] = { .labor = 1, .burial_provisions = 1, .monument_material = RESOURCE_GRANITE },
    [BUILDING_SANDSTONE_QUARRY] = { .labor = 1, .burial_provisions = 1, .monument_material = RESOURCE_SANDSTONE },
    [BUILDING_MAUSOLEUM] = { .advisors = { ADVISOR_MONUMENTS } },
    [BUILDING_HENNA_FARM] = { .advisors = { ADVISOR_POPULATION }, .labor = 1 },
    [BUILDING_ZOO] = { .advisors = { ADVISOR_ENTERTAINMENT }, .labor = 1 },
};

int32_t window_building_house_advisor(int32_t text)
{
    if (text < 0) {
        return ADVISOR_NONE;
    }
    return (text < EVOLVE_TEXT_COUNT) ? house_evolve_advisors[text] : ADVISOR_TRADE;
}

static int32_t lacks_workers(const building *b)
{
    return b->num_workers < building_model_table[b->type].laborers;
}

static int32_t output_needed_for_burial(const building *b)
{
    int32_t resource = b->output_resource;
    switch (resource) {
        case RESOURCE_GRAIN:
        case RESOURCE_GAMEMEAT:
        case RESOURCE_WEAPONS:
        case RESOURCE_POTTERY:
        case RESOURCE_BEER:
        case RESOURCE_LINEN:
        case RESOURCE_GEMS:
        case RESOURCE_LUXURY_GOODS:
        case RESOURCE_TIMBER:
        case RESOURCE_PAPYRUS:
        case RESOURCE_STONE:
        case RESOURCE_LIMESTONE:
        case RESOURCE_GRANITE:
        case RESOURCE_CHARIOTS:
        case RESOURCE_SANDSTONE:
            return scenario_data.provisions_needed[resource] != 0;
        default:
            return 0;
    }
}

static int32_t is_requested_food(int32_t resource)
{
    return resource > RESOURCE_NONE && resource < RESOURCE_MAX_FOOD;
}

static int32_t request_waiting(int32_t kind)
{
    int32_t seen = 0;
    for (scenario_event *request = scenario_request_next_active(&scenario_events, 0); request != 0;
        request = scenario_request_next_active(&scenario_events, request)) {
        if (scenario_request_is_waiting(request) == 0) {
            continue;
        }
        if (seen >= REQUESTS_LOOKED_AT) {
            return 0;
        }
        int16_t resource = request->item.value;
        if ((kind == ADVISOR_REQUEST_FOOD) ? is_requested_food(resource) : resource < RESOURCE_MAX) {
            return 1;
        }
        seen++;
    }
    return 0;
}

static int32_t work_camp_monument_needs_workers(void)
{
    for (int32_t i = 0; i < WORK_CAMP_MONUMENTS; i++) {
        int32_t def = city_player_current()->monument_def_id[i];
        if (def == 0) {
            def = scenario_data.monument_def_ids[i];
        }
        if ((uint8_t) monument_defs[def].flag != 0) {
            return 1;
        }
    }
    return 0;
}

static int32_t monuments_needed(const building_info_advisor_offer *offer, const building *b)
{
    if (offer->monument_material != RESOURCE_NONE && city_monuments_need_resource(offer->monument_material, 1) != 0) {
        return 1;
    }
    if (offer->burial_provisions && output_needed_for_burial(b)) {
        return 1;
    }
    return offer->monument_workers && work_camp_monument_needs_workers();
}

void window_building_info_advisors(int32_t *buttons)
{
    memset(buttons, 0, ADVISOR_COUNT * sizeof(buttons[0]));
    const building *b = building_info_building();
    if (b->type < 0 || b->type >= BUILDING_TYPE_COUNT) {
        return;
    }
    const building_info_advisor_offer *offer = &building_info_advisor_offers[b->type];
    for (int32_t i = 0; i < BUILDING_INFO_FIXED_ADVISORS; i++) {
        if (offer->advisors[i] != ADVISOR_NONE) {
            buttons[offer->advisors[i]] = 1;
        }
    }
    if (offer->labor && lacks_workers(b)) {
        buttons[ADVISOR_LABOR] = 1;
    }
    if (monuments_needed(offer, b)) {
        buttons[ADVISOR_MONUMENTS] = 1;
    }
    if (offer->kingdom_request != ADVISOR_REQUEST_NONE && request_waiting(offer->kingdom_request)) {
        buttons[ADVISOR_KINGDOM] = 1;
    }
    if (offer->house_evolve) {
        int32_t advisor = window_building_house_advisor(b->data.house.evolve_text_id);
        if (advisor != ADVISOR_NONE) {
            buttons[advisor] = 1;
        }
    }
}

static const int32_t *advisor_buttons_shown(void)
{
    return (const int32_t *) building_info_advisor_buttons;
}

void window_building_draw_advisor_buttons(void)
{
    const int32_t *shown = advisor_buttons_shown();
    int32_t y = building_info_y + (building_info_h * BUILDING_INFO_BLOCK) - ADVISOR_BUTTONS_FROM_BOTTOM;
    int32_t x = building_info_x + ADVISOR_BUTTONS_X;
    window_ui_redraw = 1;
    for (int32_t advisor = 0; advisor < ADVISOR_COUNT; advisor++) {
        if (shown[advisor] != 0 && tutorial_flags_b[advisor] != 0) {
            image_buttons_draw(x, y, tutorial_buttons[advisor], 1);
            x += ADVISOR_BUTTON_SPACING;
        }
    }
    g_force_refresh = 0;
}

void window_building_handle_advisor_buttons(void)
{
    const int32_t *shown = advisor_buttons_shown();
    int32_t y = building_info_y + (building_info_h * BUILDING_INFO_BLOCK) - ADVISOR_BUTTONS_FROM_BOTTOM;
    int32_t x = building_info_x + ADVISOR_BUTTONS_X;
    window_ui_redraw = 1;
    for (int32_t advisor = 0; advisor < ADVISOR_COUNT; advisor++) {
        if (shown[advisor] != 0 && tutorial_flags_b[advisor] != 0) {
            image_buttons_set_tooltip(x, y, tutorial_buttons[advisor], 1, advisor + ADVISOR_BUTTON_TOOLTIP_BASE);
            if (image_buttons_handle_mouse(x, y, tutorial_buttons[advisor], 1) != 0) {
                return;
            }
            x += ADVISOR_BUTTON_SPACING;
        }
    }
    g_force_refresh = 0;
}
