// The chief advisor's background: one row per topic (sentiment, migration, labor, food stocks and consumption,
// health, religion, finance, kingdom, military, requests, the Nile), each with the text of the city's state.

#include "window/advisor/chief.h"
#include "city/food_supply.h"
#include "city/resource_type.h"
#include "flood/flood_internal.h"
#include "game/game_internal.h"
#include "graphics/button_border.h"
#include "graphics/graphics_primitives.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "graphics/label_animation_slots.h"
#include "graphics/panel_draw.h"
#include "graphics/text_draw.h"
#include "graphics/ui_layout.h"
#include "map/map_data.h"
#include "scenario/event_helpers.h"
#include "scenario/event_manager.h"
#include "scenario/requests.h"
#include "scenario/scenario_internal.h"
#include "window/window_image_group.h"
#include <stdbool.h>
#include <stdint.h>

enum chief_advisor_row {
    CHIEF_ROW_SENTIMENT = 0,
    CHIEF_ROW_MIGRATION = 1,
    CHIEF_ROW_LABOR = 2,
    CHIEF_ROW_FOOD_STOCK = 3,
    CHIEF_ROW_FOOD_CONSUME = 4,
    CHIEF_ROW_HEALTH = 5,
    CHIEF_ROW_RELIGION = 6,
    CHIEF_ROW_FINANCE = 7,
    CHIEF_ROW_KINGDOM = 8,
    CHIEF_ROW_MILITARY = 9,
    CHIEF_ROW_REQUESTS = 10,
    CHIEF_ROW_NILE = 11,
};

static const int16_t *row_rect(enum chief_advisor_row row)
{
    return &chief_advisor_rows[row].x;
}

void window_advisor_chief_draw_background(void)
{
    advisor_x = ui_origin_x;
    advisor_y = ui_origin_y;
    advisor_height_blocks = 24;

    int32_t anim = 5 - label_animation[LABEL_SLOT_ADVISOR_OPEN];
    if (anim >= 5) {
        advisor_height_blocks = 24;
        advisor_y = ui_origin_y;
        advisor_x = ui_origin_x;
        return;
    }
    if (anim > 0) {
        graphics_draw_rect(anim * 16 + ui_origin_x, ui_origin_y + anim * 8, (40 - anim * 2) * 16, (24 - anim * 2) * 16, 0);
        return;
    }

    outer_panel_draw(ui_origin_x, ui_origin_y, 40, 24);
    image_draw(image_group(IMAGE_GROUP_ADVISOR_ICONS) + 11, advisor_x + 10, advisor_y + 10);
    lang_text_draw_ui(61, 0, advisor_x + 60, advisor_y + 12, image_group(IMAGE_GROUP_FONT_LARGE), 0, ui_origin_x, ui_origin_y);
    lang_text_draw_ui(61, 218, advisor_x + 60, advisor_y + 350, image_group(IMAGE_GROUP_FONT_BLACK), 0, ui_origin_x, ui_origin_y);
    inner_panel_draw_ui(advisor_x + 32, advisor_y + 60, 36, 17, ui_origin_x, ui_origin_y);

    for (int32_t y = 0; y < 240; y += 20) {
        image_draw(image_group(IMAGE_GROUP_BULLET), advisor_x + 40, y + 67 + advisor_y);
    }

    lang_text_draw_rect_ui(61, 1, row_rect(CHIEF_ROW_SENTIMENT), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t sentiment = city_player_current()->sentiment;
        if (sentiment < 1) {
            lang_text_draw_rect_ui(61, 20, row_rect(CHIEF_ROW_SENTIMENT), image_group(IMAGE_GROUP_FONT_RED), 0, 140, 0, ui_origin_x, ui_origin_y);
            if (city_player_current()->sentiment_low_mood_cause != 0) {
                lang_text_draw_rect_ui(
                    61, city_player_current()->sentiment_low_mood_cause + 32, row_rect(CHIEF_ROW_SENTIMENT), image_group(IMAGE_GROUP_FONT_RED), 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
            }
        } else if (sentiment > 99) {
            lang_text_draw_rect_ui(61, 31, row_rect(CHIEF_ROW_SENTIMENT), image_group(IMAGE_GROUP_FONT_GREEN), 0, 140, 0, ui_origin_x, ui_origin_y);
        } else {
            int32_t font = image_group(IMAGE_GROUP_FONT_RED);
            if (sentiment > 29) {
                font = image_group(IMAGE_GROUP_FONT_WHITE);
                if (sentiment > 49) {
                    font = image_group(IMAGE_GROUP_FONT_GREEN);
                }
            }
            lang_text_draw_rect_ui(61, sentiment / 10 + 21, row_rect(CHIEF_ROW_SENTIMENT), font, 0, 140, 0, ui_origin_x, ui_origin_y);
            if (city_player_current()->sentiment < 50 && city_player_current()->sentiment_low_mood_cause != 0) {
                lang_text_draw_rect_ui(
                    61, city_player_current()->sentiment_low_mood_cause + 31, row_rect(CHIEF_ROW_SENTIMENT), font, 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
            }
        }
    }

    lang_text_draw_rect_ui(61, 2, row_rect(CHIEF_ROW_MIGRATION), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t font = image_group(IMAGE_GROUP_FONT_RED);
        int32_t text_id;
        int32_t dir = city_player_current()->migration_direction;
        if (dir == 0) {
            if ((int32_t) (city_player_current()->kingdom_soldiers + city_player_current()->enemies) < 4) {
                if (city_player_current()->migration_newcomers < 5) {
                    if (city_player_current()->refused_immigrants_today == 0 && city_player_current()->room_in_houses > 0) {
                        if (city_player_current()->migration_percentage < 80) {
                            text_id = city_player_current()->no_immigration_cause + 46;
                        } else {
                            text_id = 44;
                            font = image_group(IMAGE_GROUP_FONT_GREEN);
                        }
                    } else {
                        text_id = 45;
                    }
                } else {
                    text_id = 44;
                    font = image_group(IMAGE_GROUP_FONT_GREEN);
                }
            } else {
                text_id = 43;
            }
        } else if (dir == 1) {
            if (city_player_current()->refused_immigrants_today == 0 && city_player_current()->room_in_houses > 0) {
                text_id = city_player_current()->no_immigration_cause + 53;
            } else {
                text_id = 52;
            }
        } else {
            text_id = 59;
            font = image_group(IMAGE_GROUP_FONT_GREEN);
        }
        lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_MIGRATION), font, 0, 140, 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_rect_ui(61, 3, row_rect(CHIEF_ROW_LABOR), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t unemp = city_player_current()->unemployment_percentage;
        int32_t font = image_group(IMAGE_GROUP_FONT_RED);
        int32_t text_id = 76;
        if (unemp < 18) {
            if (unemp < 11) {
                if (unemp < 6) {
                    if (unemp < 1) {
                        int32_t needed = city_player_current()->workers_needed;
                        if (needed < 21) {
                            if (needed < 11) {
                                font = image_group(IMAGE_GROUP_FONT_GREEN);
                                if (needed < 6) {
                                    if (needed > 0) {
                                        text_id = 83;
                                    }
                                } else {
                                    text_id = 82;
                                }
                            } else {
                                text_id = 81;
                                font = image_group(IMAGE_GROUP_FONT_WHITE);
                            }
                        } else {
                            text_id = 80;
                        }
                    } else {
                        text_id = 79;
                        font = image_group(IMAGE_GROUP_FONT_GREEN);
                    }
                } else {
                    text_id = 78;
                    font = image_group(IMAGE_GROUP_FONT_GREEN);
                }
            } else {
                text_id = 77;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            }
        } else {
            text_id = 76;
        }
        if (unemp < 6 && city_player_current()->workers_needed < 6) {
            lang_text_draw_rect_ui(61, 84, row_rect(CHIEF_ROW_LABOR), image_group(IMAGE_GROUP_FONT_GREEN), 0, 140, 0, ui_origin_x, ui_origin_y);
        } else {
            lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_LABOR), font, 0, 140, 0, ui_origin_x, ui_origin_y);
            if (unemp < 1) {
                if (city_player_current()->workers_needed > 0) {
                    text_draw_number_rect(city_player_current()->workers_needed, NUMBER_PREFIX_NONE, postfix_none, row_rect(CHIEF_ROW_LABOR), font, 0, g_text_width + 140, 0);
                    lang_text_draw_rect_ui(8, 13, row_rect(CHIEF_ROW_LABOR), font, 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
                }
            } else {
                text_draw_number_rect(city_player_current()->unemployment_percentage, NUMBER_PREFIX_NONE, pct_postfix, row_rect(CHIEF_ROW_LABOR), font, 0, g_text_width + 140, 0);
            }
        }
    }

    lang_text_draw_rect_ui(61, 4, row_rect(CHIEF_ROW_FOOD_STOCK), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
        int32_t group = 61;
        int32_t text_id;
        int32_t x_off = 140;
        if (scenario_data.kingdom_supplies_grain != 0) {
            text_id = 94;
        } else if (city_player_current()->food_supply_months < 1) {
            text_id = 95;
            font = image_group(IMAGE_GROUP_FONT_RED);
        } else if ((city_player_current()->food_stored_total * 100) / city_player_current()->food_needed_per_month < 50) {
            text_id = 96;
            font = image_group(IMAGE_GROUP_FONT_WHITE);
        } else if (city_player_current()->food_supply_months < 1) {
            text_id = 97;
        } else {
            lang_text_draw_rect_ui(61, 98, row_rect(CHIEF_ROW_FOOD_STOCK), image_group(IMAGE_GROUP_FONT_GREEN), 0, 140, 0, ui_origin_x, ui_origin_y);
            text_draw_number_rect(city_player_current()->food_supply_months, NUMBER_PREFIX_NONE, postfix_none, row_rect(CHIEF_ROW_FOOD_STOCK), image_group(IMAGE_GROUP_FONT_GREEN), 0, g_text_width + 140, 0);
            x_off = g_text_width + 140;
            text_id = 5;
            group = 8;
            font = image_group(IMAGE_GROUP_FONT_GREEN);
        }
        lang_text_draw_rect_ui(group, text_id, row_rect(CHIEF_ROW_FOOD_STOCK), font, 0, x_off, 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_rect_ui(61, 5, row_rect(CHIEF_ROW_FOOD_CONSUME), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
        int32_t text_id = 18;
        if (city_player_current()->population != 0) {
            int32_t pct = (city_resource_food_supply_percentage() * 100) / city_player_current()->population;
            if (pct > 150) {
                text_id = 13;
            } else if (pct > 110) {
                text_id = 14;
            } else if (pct > 95) {
                text_id = 15;
            } else if (pct > 85) {
                text_id = 16;
            } else if (pct > 70) {
                text_id = 17;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            } else {
                text_id = 18;
                font = image_group(IMAGE_GROUP_FONT_RED);
            }
        } else {
            font = image_group(IMAGE_GROUP_FONT_RED);
        }
        lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_FOOD_CONSUME), font, 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_rect_ui(61, 6, row_rect(CHIEF_ROW_HEALTH), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t health = city_player_current()->health;
        int32_t font = image_group(IMAGE_GROUP_FONT_RED);
        if (health > 19) {
            font = image_group(IMAGE_GROUP_FONT_WHITE);
            if (health > 29) {
                font = image_group(IMAGE_GROUP_FONT_GREEN);
            }
        }
        lang_text_draw_rect_ui(61, health / 10 + 103, row_rect(CHIEF_ROW_HEALTH), font, 0, 140, 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_rect_ui(61, 7, row_rect(CHIEF_ROW_RELIGION), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t low_count = 0;
        int32_t min_mood = 100;
        int32_t i;
        for (i = 0; i < 5; i++) {
            int8_t mood = city_player_current()->god_target_mood[i];
            if (mood < 30) {
                low_count++;
            }
            if (mood < min_mood) {
                min_mood = mood;
            }
        }
        int32_t high_count = 0;
        for (i = 0; i < 5; i++) {
            if (city_player_current()->god_target_mood[i] > 70) {
                high_count++;
            }
        }
        int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
        int32_t text_id;
        if (low_count == 0) {
            if (high_count == 0) {
                text_id = 133;
            } else if (high_count == 1) {
                text_id = 134;
            } else if (high_count == 2) {
                text_id = 135;
            } else {
                text_id = (high_count != 3) + 136;
            }
        } else if (low_count >= 4) {
            if (min_mood < 20) {
                text_id = 126;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            } else {
                text_id = 125;
            }
        } else if (low_count == 3) {
            if (min_mood < 20) {
                text_id = 128;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            } else {
                text_id = 127;
            }
        } else if (low_count == 2) {
            if (min_mood < 20) {
                text_id = 130;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            } else {
                text_id = 129;
            }
        } else {
            if (min_mood < 20) {
                text_id = 132;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            } else {
                text_id = 131;
            }
        }
        lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_RELIGION), font, 0, 140, 0, ui_origin_x, ui_origin_y);
    }

    lang_text_draw_rect_ui(61, 8, row_rect(CHIEF_ROW_FINANCE), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t money_delta = -1;
        int32_t font = image_group(IMAGE_GROUP_FONT_WHITE);
        int32_t text_id;
        int32_t uncollected = city_player_current()->yearly_uncollected_nobles + city_player_current()->yearly_uncollected_plebs;
        int32_t collected = city_player_current()->yearly_collected_nobles + city_player_current()->yearly_collected_plebs;
        if (uncollected == 0 || collected / uncollected > 0) {
            int32_t treasury = city_player_current()->treasury;
            int32_t last = city_player_current()->balance_last_year;
            if (last < treasury) {
                money_delta = treasury - last;
                text_id = 152;
                font = image_group(IMAGE_GROUP_FONT_GREEN);
            } else if (treasury < last) {
                money_delta = last - treasury;
                text_id = 154;
            } else {
                text_id = 153;
                font = image_group(IMAGE_GROUP_FONT_GREEN);
            }
        } else {
            text_id = 151;
        }
        lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_FINANCE), font, 0, 140, 0, ui_origin_x, ui_origin_y);
        if (money_delta != -1) {
            text_draw_number_rect(money_delta, NUMBER_PREFIX_NONE, money_postfix, row_rect(CHIEF_ROW_FINANCE), font, 0, g_text_width + 140, 0);
        }
    }

    lang_text_draw_rect_ui(61, 9, row_rect(CHIEF_ROW_KINGDOM), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t amount = -1;
        int32_t font;
        int32_t text_id;
        int32_t months = city_player_current()->crimes_this_month;
        if (months < 10) {
            font = image_group(IMAGE_GROUP_FONT_GREEN);
            if (months == 0) {
                text_id = 163;
            } else if (months < 2) {
                amount = city_player_current()->gold_stolen_this_month;
                text_id = 162;
            } else if (months < 5) {
                amount = city_player_current()->gold_stolen_this_month;
                text_id = 161;
            } else {
                amount = city_player_current()->gold_stolen_this_month;
                text_id = 160;
                font = image_group(IMAGE_GROUP_FONT_WHITE);
            }
        } else {
            amount = city_player_current()->gold_stolen_this_month;
            text_id = 159;
            font = image_group(IMAGE_GROUP_FONT_RED);
        }
        lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_KINGDOM), font, 0, 140, 0, ui_origin_x, ui_origin_y);
        if (amount != -1) {
            text_draw_number_rect(amount, NUMBER_PREFIX_NONE, postfix_none, row_rect(CHIEF_ROW_KINGDOM), font, 0, g_text_width + 140, 0);
            lang_text_draw_rect_ui(61, 164, row_rect(CHIEF_ROW_KINGDOM), font, 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
        }
    }

    {
        uint8_t has_waiting_troops = 0;
        uint8_t has_received = 0;
        uint8_t has_traveling = 0;
        for (scenario_event *e = scenario_request_next_active(&scenario_events, 0); e != 0; e = scenario_request_next_active(&scenario_events, e)) {
            if (e->item.value != RESOURCE_TROOPS) {
                continue;
            }
            if (scenario_request_is_waiting(e)) {
                has_waiting_troops = 1;
            }
            if (e->state == 5) {
                has_received = 1;
            }
            if (e->state == 6 || e->state == 7) {
                has_traveling = 1;
            }
        }
        lang_text_draw_rect_ui(61, 10, row_rect(CHIEF_ROW_MILITARY), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        {
            int32_t font = image_group(IMAGE_GROUP_FONT_GREEN);
            int32_t text_id;
            if (city_player_current()->kingdom_soldiers != 0) {
                text_id = 174;
                font = image_group(IMAGE_GROUP_FONT_RED);
            } else if (city_player_current()->enemies != 0) {
                text_id = 173;
                font = image_group(IMAGE_GROUP_FONT_RED);
            } else if (scenario_invasion_running() != 0) {
                text_id = 172;
                font = image_group(IMAGE_GROUP_FONT_RED);
            } else if (has_traveling || has_received) {
                text_id = 176;
            } else if (has_waiting_troops) {
                text_id = 175;
            } else {
                text_id = (city_player_current()->soldiers >= 0) ? 171 : 170;
            }
            lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_MILITARY), font, 0, g_text_width + 140, 0, ui_origin_x, ui_origin_y);
        }
    }

    {
        int32_t count = 0;
        for (scenario_event *e = scenario_request_next_active(&scenario_events, 0); e != 0; e = scenario_request_next_active(&scenario_events, e)) {
            if (!scenario_request_is_waiting(e)) {
                continue;
            }
            if (count > 4) {
                break;
            }
            if (e->item.value != 0) {
                count++;
            }
        }
        lang_text_draw_rect_ui(61, 11, row_rect(CHIEF_ROW_REQUESTS), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
        g_text_width = 0;
        {
            int32_t text_id;
            if (count == 0) {
                text_id = 187;
            } else if (count < 3) {
                text_id = 186;
            } else {
                text_id = (count < 5) + 184;
            }
            lang_text_draw_rect_ui(61, text_id, row_rect(CHIEF_ROW_REQUESTS), image_group(IMAGE_GROUP_FONT_GREEN), 0, 140, 0, ui_origin_x, ui_origin_y);
        }
    }

    lang_text_draw_rect_ui(61, 12, row_rect(CHIEF_ROW_NILE), image_group(IMAGE_GROUP_FONT_WHITE), 0, 0, 0, ui_origin_x, ui_origin_y);
    g_text_width = 0;
    {
        int32_t quality = floodplain_settings_get_quality(&floods_data.settings);
        int32_t level = (quality + 19) / 20;
        int32_t font = image_group(IMAGE_GROUP_FONT_RED);
        if (level != 0) {
            font = image_group(IMAGE_GROUP_FONT_WHITE);
            if (level != 1) {
                font = image_group(IMAGE_GROUP_FONT_GREEN);
            }
        }
        lang_text_draw_rect_ui(61, level + 192, row_rect(CHIEF_ROW_NILE), font, 0, 140, 0, ui_origin_x, ui_origin_y);
        if (level != 0) {
            int32_t text_id = (floods_data.settings.season_initial % 30) / 15 + 194 + floods_data.season_month * 2;
            lang_text_draw_rect_ui(61, text_id, &row_nile2.chief_nile_row[0].x, image_group(IMAGE_GROUP_FONT_GREEN), 0, 140, 0, ui_origin_x, ui_origin_y);
        }
    }
}
