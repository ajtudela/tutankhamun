// House evolution requirements: whether a house has every service and good its level (or the next) asks for,
// counting the missing ones in the demand counters, and the evolve status text of the info window.

#include "building/house_evolve.h"
#include "city/city_player.h"

enum { SECOND_LUXURY_KINDS = 2 };

enum evolve_status { EVOLVE_STATUS_DEVOLVE = -1, EVOLVE_STATUS_STAY = 0, EVOLVE_STATUS_EVOLVE = 1 };

int32_t house_has_required_goods_and_services(int32_t building_id, int32_t for_upgrade)
{
    building *b = building_get(building_id);
    city_player *player = city_player_current();
    int32_t level = b->subtype;
    if (for_upgrade) {
        level++;
    }
    const house_model *m = &house_models[level];
    int32_t size = b->size;
    int32_t religion = m->religion;
    int32_t dentist = m->dentist;
    int32_t courthouse = m->courthouse;
    int32_t health = m->health;
    int32_t entertainment = m->entertainment;
    int32_t education = m->education;
    int32_t food_types = m->food_types;
    int32_t pottery = house_good_requirement(m->pottery, size);
    int32_t linen = house_good_requirement(m->linen, size);
    int32_t jewelry = house_good_requirement(m->jewelry, size);
    int32_t beer = house_good_requirement(m->beer, size);

    if (!b->has_water_access) {
        if (m->water >= HOUSE_WATER_SUPPLY) {
            player->houses_missing_water_supply++;
            return 0;
        }
        if (m->water == HOUSE_WATER_WELL && !b->has_well_access) {
            player->houses_missing_well++;
            return 0;
        }
    }
    if (b->data.house.entertainment < entertainment) {
        if (b->data.house.entertainment == 0) {
            player->houses_missing_entertainment++;
        } else {
            player->houses_missing_more_entertainment++;
        }
        return 0;
    }
    if (b->data.house.education < education) {
        if (b->data.house.education != 0) {
            player->houses_missing_more_education++;
        } else {
            player->houses_missing_education++;
        }
        return 0;
    }
    if (education == HOUSE_EDUCATION_SCHOOL_AND_LIBRARY) {
        player->houses_requiring_school++;
        player->houses_requiring_library++;
    } else if (education == HOUSE_EDUCATION_SCHOOL_OR_LIBRARY) {
        player->houses_requiring_school++;
    }
    if (b->data.house.num_gods < religion) {
        if (religion == 1) {
            player->houses_missing_religion++;
            return 0;
        }
        if (religion == 2) {
            player->houses_missing_second_religion++;
            return 0;
        }
        if (religion == 3) {
            player->houses_missing_third_religion++;
            return 0;
        }
    } else if (religion > 0) {
        player->houses_requiring_religion++;
    }
    if (b->data.house.dentist < dentist) {
        player->houses_missing_dentist++;
        return 0;
    }
    if (dentist == 1) {
        player->houses_requiring_dentist++;
    }
    if (b->data.house.courthouse < courthouse) {
        if (b->data.house.courthouse != 0) {
            player->houses_missing_courthouse++;
        } else {
            player->houses_missing_any_courthouse++;
        }
        return 0;
    }
    if (courthouse == 2) {
        player->houses_requiring_courthouse++;
        player->houses_requiring_more_courthouse++;
    } else if (courthouse == 1) {
        player->houses_requiring_courthouse++;
    }
    if (b->data.house.health < health) {
        if (health >= 2) {
            player->houses_missing_more_health++;
        } else {
            player->houses_missing_health++;
        }
        return 0;
    }
    if (health > 0) {
        player->houses_requiring_health++;
    }
    if (house_food_types_available(b) < food_types) {
        player->houses_missing_food++;
        return 0;
    }
    if (b->data.house.inventory[INVENTORY_POTTERY] < pottery) {
        return 0;
    }
    if (beer != 0 && b->data.house.inventory[INVENTORY_BEER] <= 0) {
        return 0;
    }
    if (b->data.house.inventory[INVENTORY_LINEN] < linen) {
        return 0;
    }
    if (b->data.house.inventory[INVENTORY_JEWELRY] < jewelry) {
        return 0;
    }
    if (jewelry >= SECOND_LUXURY_KINDS && player->luxury_goods_kinds < SECOND_LUXURY_KINDS) {
        player->houses_missing_second_luxury++;
        return 0;
    }
    return 1;
}

static int32_t entertainment_text(int32_t needed)
{
    if (needed < 10) {
        return EVOLVE_TEXT_ENTERTAINMENT_BELOW_10;
    }
    if (needed < 25) {
        return EVOLVE_TEXT_ENTERTAINMENT_BELOW_25;
    }
    if (needed < 50) {
        return EVOLVE_TEXT_ENTERTAINMENT_BELOW_50;
    }
    if (needed < 80) {
        return EVOLVE_TEXT_ENTERTAINMENT_BELOW_80;
    }
    return EVOLVE_TEXT_ENTERTAINMENT_HIGH;
}

void house_update_evolve_text(int32_t building_id)
{
    building *b = building_get(building_id);
    const city_player *player = city_player_current();
    int32_t level = b->subtype;
    const house_model *m = &house_models[level];
    int32_t desirability = b->desirability;
    enum evolve_status status;
    if (desirability <= m->devolve_desirability) {
        status = EVOLVE_STATUS_DEVOLVE;
    } else if (desirability >= m->evolve_desirability) {
        status = EVOLVE_STATUS_EVOLVE;
    } else {
        status = EVOLVE_STATUS_STAY;
    }
    int32_t water = m->water;
    int32_t entertainment = m->entertainment;
    int32_t education = m->education;
    int32_t religion = m->religion;
    int32_t dentist = m->dentist;
    int32_t courthouse = m->courthouse;
    int32_t health = m->health;
    int32_t food_types = m->food_types;
    int32_t size = b->size;
    int32_t pottery = house_good_requirement(m->pottery, size);
    int32_t linen = house_good_requirement(m->linen, size);
    int32_t jewelry = house_good_requirement(m->jewelry, size);
    int32_t beer = house_good_requirement(m->beer, size);
    int32_t foods = house_food_types_available(b);

    if (status == EVOLVE_STATUS_DEVOLVE) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_DEVOLVING_DESIRABILITY;
        return;
    }
    if (water == HOUSE_WATER_WELL) {
        if (!b->has_water_access && !b->has_well_access) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NO_WELL;
            return;
        }
    } else if (water == HOUSE_WATER_SUPPLY && !b->has_water_access) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NO_WATER_SUPPLY;
        return;
    }
    int32_t house_entertainment = b->data.house.entertainment;
    if (house_entertainment < entertainment) {
        b->data.house.evolve_text_id = (house_entertainment == 0) ? EVOLVE_TEXT_NO_ENTERTAINMENT : entertainment_text(entertainment);
        return;
    }
    if (foods < food_types) {
        if (food_types == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_FOOD_TYPES_1;
            return;
        }
        if (food_types == 2) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_FOOD_TYPES_2;
            return;
        }
        if (food_types == 3) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_FOOD_TYPES_3;
            return;
        }
    }
    if (b->data.house.education < education) {
        if (education == HOUSE_EDUCATION_SCHOOL_OR_LIBRARY) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NO_EDUCATION;
            return;
        }
        if (education == HOUSE_EDUCATION_SCHOOL_AND_LIBRARY) {
            if (b->data.house.school) {
                b->data.house.evolve_text_id = EVOLVE_TEXT_NO_LIBRARY;
                return;
            }
            if (b->data.house.library) {
                b->data.house.evolve_text_id = EVOLVE_TEXT_NO_SCHOOL;
                return;
            }
        } else if (education == HOUSE_EDUCATION_HIGHER) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NO_HIGHER_EDUCATION;
            return;
        }
    }
    int32_t house_courthouse = b->data.house.courthouse;
    if (house_courthouse < courthouse) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_COURTHOUSE;
        return;
    }
    if (b->data.house.inventory[INVENTORY_POTTERY] < pottery) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_POTTERY;
        return;
    }
    if (b->data.house.num_gods < religion) {
        if (religion == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_RELIGION_1;
            return;
        }
        if (religion == 2) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_RELIGION_2;
            return;
        }
        if (religion == 3) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_RELIGION_3;
            return;
        }
    }
    int32_t house_dentist = b->data.house.dentist;
    if (house_dentist < dentist) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_DENTIST;
        return;
    }
    if (b->data.house.health < health) {
        if (health == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_HEALTH_1;
            return;
        }
        b->data.house.evolve_text_id = b->data.house.physician ? EVOLVE_TEXT_HEALTH_2_HAS_PHYSICIAN : EVOLVE_TEXT_HEALTH_2;
        return;
    }
    if (b->data.house.inventory[INVENTORY_BEER] < beer) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_BEER;
        return;
    }
    if (b->data.house.inventory[INVENTORY_LINEN] < linen) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_LINEN;
        return;
    }
    if (b->data.house.inventory[INVENTORY_JEWELRY] < jewelry) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_JEWELRY;
        return;
    }
    if (jewelry >= SECOND_LUXURY_KINDS && player->luxury_goods_kinds < SECOND_LUXURY_KINDS) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_SECOND_JEWELRY;
        return;
    }
    if (level >= HOUSE_LEVEL_MAX) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_TOP_LEVEL;
        return;
    }

    const house_model *n = &house_models[level + 1];
    entertainment = n->entertainment;
    education = n->education;
    religion = n->religion;
    water = n->water;
    food_types = n->food_types;
    pottery = house_good_requirement(n->pottery, size);
    linen = house_good_requirement(n->linen, size);
    jewelry = house_good_requirement(n->jewelry, size);
    beer = house_good_requirement(n->beer, size);
    if (status == EVOLVE_STATUS_STAY) {
        b->data.house.evolve_text_id = house_worst_desirability_building ? EVOLVE_TEXT_NEXT_DESIRABILITY_E : EVOLVE_TEXT_NEXT_LEVEL;
        return;
    }
    if (water == HOUSE_WATER_WELL) {
        if (!b->has_water_access && !b->has_well_access) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_NO_WELL;
            return;
        }
    } else if (water == HOUSE_WATER_SUPPLY && !b->has_water_access) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_NO_WATER_SUPPLY;
        return;
    }
    if (house_entertainment < entertainment) {
        int32_t next_text = EVOLVE_TEXT_NEXT_LEVEL + entertainment_text(entertainment);
        b->data.house.evolve_text_id = (house_entertainment == 0) ? EVOLVE_TEXT_NEXT_NO_ENTERTAINMENT : next_text;
        return;
    }
    if (foods < food_types) {
        if (food_types == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_FOOD_TYPES_1;
            return;
        }
        if (food_types == 2) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_FOOD_TYPES_2;
            return;
        }
        if (food_types == 3) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_FOOD_TYPES_3;
            return;
        }
    }
    if (b->data.house.education < education) {
        if (education == HOUSE_EDUCATION_SCHOOL_OR_LIBRARY) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_NO_EDUCATION;
            return;
        }
        if (education == HOUSE_EDUCATION_SCHOOL_AND_LIBRARY) {
            if (b->data.house.school) {
                b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_NO_LIBRARY;
                return;
            }
            if (b->data.house.library) {
                b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_NO_SCHOOL;
                return;
            }
        }
    }
    if (house_courthouse < n->courthouse) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_COURTHOUSE;
        return;
    }
    if (b->data.house.inventory[INVENTORY_POTTERY] < pottery) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_POTTERY;
        return;
    }
    if (b->data.house.num_gods < religion) {
        if (religion == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_RELIGION_1;
            return;
        }
        if (religion == 2) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_RELIGION_2;
            return;
        }
        if (religion == 3) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_RELIGION_3;
            return;
        }
    }
    if (house_dentist < n->dentist) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_DENTIST;
        return;
    }
    if (b->data.house.health < n->health) {
        if (n->health == 1) {
            b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_HEALTH_1;
            return;
        }
        b->data.house.evolve_text_id = b->data.house.physician ? EVOLVE_TEXT_NEXT_HEALTH_2_HAS_PHYSICIAN : EVOLVE_TEXT_NEXT_HEALTH_2;
        return;
    }
    if (b->data.house.inventory[INVENTORY_BEER] < beer) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_BEER;
        return;
    }
    if (b->data.house.inventory[INVENTORY_LINEN] < linen) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_LINEN;
        return;
    }
    if (b->data.house.inventory[INVENTORY_JEWELRY] < jewelry) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_JEWELRY;
        return;
    }
    if (jewelry >= SECOND_LUXURY_KINDS && player->luxury_goods_kinds < SECOND_LUXURY_KINDS) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_NEXT_SECOND_JEWELRY;
        return;
    }
    b->data.house.evolve_text_id = EVOLVE_TEXT_EVOLVING;
    if (b->data.house.no_space_to_expand == 1) {
        b->data.house.evolve_text_id = EVOLVE_TEXT_EVOLVING_NO_SPACE;
    }
}
