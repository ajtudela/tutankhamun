// The built-in tables of the scenario events (scenario_rules.h); the config/*.json files of the same names replace
// them when present.

#include "scenario/scenario_rules.h"

request_message_rule request_message_rules[REQUEST_SUBTYPES] = {
    [0] = {
        .announcement = { 53, 52 }, .completed = { 47, 46 }, .refused = { 51, 50 },
        .too_late = { 49, 48 }, .defeat = { -1, -1 }, .completed_group = 8, .refused_group = 10,
        .too_late_group = 9, .defeat_group = -1, .reason_column = 2,
    },
    [1] = {
        .announcement = { 17, 16 }, .completed = { 9, 8 }, .refused = { 13, 12 },
        .too_late = { 11, 10 }, .defeat = { 15, 14 }, .completed_group = 0, .refused_group = 3,
        .too_late_group = 2, .defeat_group = 1, .reason_column = 0,
    },
    [2] = {
        .announcement = { 35, 34 }, .completed = { 27, 26 }, .refused = { 31, 30 },
        .too_late = { 29, 28 }, .defeat = { 33, 32 }, .completed_group = 4, .refused_group = 7,
        .too_late_group = 6, .defeat_group = 5, .reason_column = 1,
    },
    [3] = {
        .announcement = { 71, 70 }, .completed = { 65, 64 }, .refused = { 69, 68 },
        .too_late = { 67, 66 }, .defeat = { -1, -1 }, .completed_group = 11, .refused_group = 13,
        .too_late_group = 12, .defeat_group = -1, .reason_column = 3,
    },
    [4] = {
        .announcement = { 89, 88 }, .completed = { 83, 82 }, .refused = { 87, 86 },
        .too_late = { 85, 84 }, .defeat = { -1, -1 }, .completed_group = 14, .refused_group = 16,
        .too_late_group = 15, .defeat_group = -1, .reason_column = 4,
    },
    [5] = {
        .announcement = { 107, 106 }, .completed = { 101, 100 }, .refused = { 105, 104 },
        .too_late = { 103, 102 }, .defeat = { -1, -1 }, .completed_group = 17, .refused_group = 19,
        .too_late_group = 18, .defeat_group = -1, .reason_column = 5,
    },
    [6] = {
        .announcement = { 125, 124 }, .completed = { 119, 118 }, .refused = { 123, 122 },
        .too_late = { 121, 120 }, .defeat = { -1, -1 }, .completed_group = 20, .refused_group = 22,
        .too_late_group = 21, .defeat_group = -1, .reason_column = 6,
    },
};

earthquake_step earthquake_steps[EARTHQUAKE_STEPS] = {
    [0] = { .front = 0, .dx = 0, .dy = -1 },
    [1] = { .front = 1, .dx = 1, .dy = 0 },
    [2] = { .front = 2, .dx = 0, .dy = 1 },
    [3] = { .front = 3, .dx = -1, .dy = 0 },
    [4] = { .front = 0, .dx = 0, .dy = -1 },
    [5] = { .front = 0, .dx = -1, .dy = 0 },
    [6] = { .front = 0, .dx = 1, .dy = 0 },
    [7] = { .front = 1, .dx = 1, .dy = 0 },
    [8] = { .front = 1, .dx = 0, .dy = -1 },
    [9] = { .front = 1, .dx = 0, .dy = 1 },
    [10] = { .front = 2, .dx = 0, .dy = 1 },
    [11] = { .front = 2, .dx = -1, .dy = 0 },
    [12] = { .front = 2, .dx = 1, .dy = 0 },
    [13] = { .front = 3, .dx = -1, .dy = 0 },
    [14] = { .front = 3, .dx = 0, .dy = -1 },
    [15] = { .front = 3, .dx = 0, .dy = 1 },
};

troops_loss_bracket troops_loss_brackets[TROOPS_LOSS_BRACKETS] = {
    [0] = { .below_percent = 10, .losses = 70 },
    [1] = { .below_percent = 25, .losses = 50 },
    [2] = { .below_percent = 50, .losses = 25 },
    [3] = { .below_percent = 75, .losses = 15 },
    [4] = { .below_percent = 100, .losses = 10 },
    [5] = { .below_percent = 150, .losses = 5 },
    [6] = { .below_percent = INT32_MAX, .losses = 0 },
};

int32_t reason_phrase_variant_texts[REASON_PHRASE_VARIANT_TEXTS] = {
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 46, 47, 48, 49, 50, 51, 52, 53, 64, 65, 66, 67, 68, 69, 70, 71, 82, 83, 84,
    85, 86, 87, 88, 89, 100, 101, 102, 103, 104, 105, 106, 107, 118, 119, 120, 121, 122, 123, 124, 125, 128, 129, 132, 133, 136, 137, 140, 141, 144, 145,
    148, 149, 152, 153, 156, 157, 160, 161, 169, 170, 179, 180, 189, 190, 193, 194, 197, 198, 201, 202, 205, 206, 209, 210, 213, 214, 217, 218, 221, 222,
    225, 226, 229, 230, 233, 234, 237, 238, 241, 242, 245, 246, 249, 250, 253, 254, 262, 263, 286, 287, 288, 289, 290, 291, 292, 293,
};

event_type_property event_type_properties[EVENT_TYPE_COUNT] = {
    [EVENT_TYPE_NONE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_REQUEST] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 38, .item_text_group = 23, .has_amount = 1,
    },
    [EVENT_TYPE_INVASION] = {
        .uses_city = 1, .is_invasion = 1, .shows_items = 1, .uses_resource = 0, .item_min = 1, .item_max = 5, .item_text_group = 34, .has_amount = 1,
    },
    [EVENT_TYPE_EARTHQUAKE] = {
        .uses_city = 1, .is_invasion = 1, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_REVOLT] = {
        .uses_city = 1, .is_invasion = 1, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_CHANGE_PHARAOH] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_SEA_TRADE_PROBLEM] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_LAND_TRADE_PROBLEM] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_WAGE_INCREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_WAGE_DECREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_CONTAMINATED_WATER] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_GOLD_MINE_COLLAPSE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_CLAY_PIT_FLOOD] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_DEMAND_INCREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 36, .item_text_group = 23, .has_amount = 0,
    },
    [EVENT_TYPE_DEMAND_DECREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 36, .item_text_group = 23, .has_amount = 0,
    },
    [EVENT_TYPE_PRICE_INCREASE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 36, .item_text_group = 23, .has_amount = 1,
    },
    [EVENT_TYPE_PRICE_DECREASE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 36, .item_text_group = 23, .has_amount = 1,
    },
    [EVENT_TYPE_REPUTATION_INCREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_REPUTATION_DECREASE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_CITY_STATUS_CHANGE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_MESSAGE] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_FLOOD_WORSE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_FLOOD_BETTER] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_GIFT_FROM_PHARAOH] = {
        .uses_city = 1, .is_invasion = 0, .shows_items = 1, .uses_resource = 1, .item_min = 1, .item_max = 38, .item_text_group = 23, .has_amount = 1,
    },
    [EVENT_TYPE_LOCUSTS] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_FROGS] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_HAILSTORM] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 0,
    },
    [EVENT_TYPE_BLOOD_RIVER] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_CRIME_WAVE] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
    [EVENT_TYPE_MUMMY] = {
        .uses_city = 0, .is_invasion = 0, .shows_items = 0, .uses_resource = 0, .item_min = 0, .item_max = 0, .item_text_group = -1, .has_amount = 1,
    },
};

event_type_text event_type_texts[EVENT_TYPE_COUNT] = {
    [EVENT_TYPE_NONE] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_REQUEST] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_INVASION] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_EARTHQUAKE] = { .announcement = 218, .completed = 217, .reason_group = 43, .reason_column = 27, .first_phrase = 216, .second_phrase = 215 },
    [EVENT_TYPE_REVOLT] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_CHANGE_PHARAOH] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_SEA_TRADE_PROBLEM] = {
        .announcement = 222, .completed = 221, .reason_group = 44, .reason_column = 28, .first_phrase = 220, .second_phrase = 219,
    },
    [EVENT_TYPE_LAND_TRADE_PROBLEM] = {
        .announcement = 226, .completed = 225, .reason_group = 45, .reason_column = 29, .first_phrase = 224, .second_phrase = 223,
    },
    [EVENT_TYPE_WAGE_INCREASE] = { .announcement = 230, .completed = 229, .reason_group = 41, .reason_column = 25, .first_phrase = 228, .second_phrase = 227 },
    [EVENT_TYPE_WAGE_DECREASE] = { .announcement = 234, .completed = 233, .reason_group = 42, .reason_column = 26, .first_phrase = 232, .second_phrase = 231 },
    [EVENT_TYPE_CONTAMINATED_WATER] = {
        .announcement = 238, .completed = 237, .reason_group = 46, .reason_column = 30, .first_phrase = 236, .second_phrase = 235,
    },
    [EVENT_TYPE_GOLD_MINE_COLLAPSE] = {
        .announcement = 242, .completed = 241, .reason_group = 47, .reason_column = 31, .first_phrase = 240, .second_phrase = 239,
    },
    [EVENT_TYPE_CLAY_PIT_FLOOD] = { .announcement = -1, .completed = -1, .reason_group = 47, .reason_column = 31, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_DEMAND_INCREASE] = {
        .announcement = 210, .completed = 209, .reason_group = 37, .reason_column = 21, .first_phrase = 208, .second_phrase = 207,
    },
    [EVENT_TYPE_DEMAND_DECREASE] = {
        .announcement = 214, .completed = 213, .reason_group = 38, .reason_column = 22, .first_phrase = 212, .second_phrase = 211,
    },
    [EVENT_TYPE_PRICE_INCREASE] = { .announcement = 202, .completed = 201, .reason_group = 39, .reason_column = 23, .first_phrase = 200, .second_phrase = 199 },
    [EVENT_TYPE_PRICE_DECREASE] = { .announcement = 206, .completed = 205, .reason_group = 40, .reason_column = 24, .first_phrase = 204, .second_phrase = 203 },
    [EVENT_TYPE_REPUTATION_INCREASE] = {
        .announcement = 194, .completed = 193, .reason_group = 35, .reason_column = 19, .first_phrase = 192, .second_phrase = 191,
    },
    [EVENT_TYPE_REPUTATION_DECREASE] = {
        .announcement = 198, .completed = 197, .reason_group = 36, .reason_column = 20, .first_phrase = 196, .second_phrase = 195,
    },
    [EVENT_TYPE_CITY_STATUS_CHANGE] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_MESSAGE] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_FLOOD_WORSE] = { .announcement = 250, .completed = 249, .reason_group = 48, .reason_column = 32, .first_phrase = 248, .second_phrase = 247 },
    [EVENT_TYPE_FLOOD_BETTER] = { .announcement = 254, .completed = 253, .reason_group = 49, .reason_column = 33, .first_phrase = 252, .second_phrase = 251 },
    [EVENT_TYPE_GIFT_FROM_PHARAOH] = { .announcement = -1, .completed = -1, .reason_group = 53, .reason_column = 35, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_LOCUSTS] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_FROGS] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_HAILSTORM] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_BLOOD_RIVER] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_CRIME_WAVE] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
    [EVENT_TYPE_MUMMY] = { .announcement = -1, .completed = -1, .reason_group = -1, .reason_column = -1, .first_phrase = -1, .second_phrase = -1 },
};

invasion_warning invasion_warnings[INVASION_WARNINGS] = {
    [0] = { .months_left = 0, .phrase = { 0xB7, 0xAD, 0xA3, 0x100 }, .message = { 0x77, 0x81, 0x7E, 0x77 } },
    [1] = { .months_left = 24, .phrase = { 0xB8, 0xAE, 0xA4, 0x101 }, .message = { 0x77, 0x81, 0x7E, 0x77 } },
    [2] = { .months_left = 12, .phrase = { 0xB9, 0xAF, 0xA5, 0x102 }, .message = { 0x76, 0x80, 0x7D, 0x76 } },
    [3] = { .months_left = 6, .phrase = { 0xBA, 0xB0, 0xA6, 0x103 }, .message = { 0x71, 0x7F, 0x7C, 0x71 } },
    [4] = { .months_left = 1, .phrase = { 0xBB, 0xB1, 0xA7, 0x104 }, .message = { 0x71, 0x7F, 0x7C, 0x71 } },
};
