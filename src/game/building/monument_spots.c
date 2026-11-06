// Monument-wide helpers: the monument type checks, the daily completion check and message, the material still
// needed by monuments not yet placed, loading the animation scripts and the animation tick of all work spots.
//
// The monument checks have three gaps:
// - the large obelisk is missing from the monument list, so it counts as not a monument;
// - only the small royal tomb also needs a created sequence of at least -1 to complete;
// - an unknown pyramid style still completes, with its phrases left at -1.

#include "building/monument_spots.h"
#include "building/abu_simbel.h"
#include "building/building_type.h"
#include "building/caesareum.h"
#include "building/construction_place.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/mausoleum.h"
#include "building/monument.h"
#include "building/monument_checks.h"
#include "building/monument_object.h"
#include "building/obelisk.h"
#include "building/pyramid.h"
#include "building/pyramid_blocks.h"
#include "building/royal_tomb.h"
#include "building/sphinx.h"
#include "building/sun_temple.h"
#include "city/city_player.h"
#include "city/message.h"
#include "city/resource_type.h"
#include "figure/figure.h"
#include "figure/figure_action.h"
#include "figure/figure_core.h"
#include "figure/figure_type.h"
#include "game/game_data.h"
#include "map/grid.h"
#include "platform/log.h"
#include "widget/city_view.h"

void monuments_animate_all_spots(void)
{
    sphinx_animate_spots();
    obelisk_animate_spots();
    sun_temple_animate_spots();
    caesareum_animate_spots();
    royal_tomb_animate_spots();
    library_animate_spots();
    abu_simbel_animate_spots();
    lighthouse_animate_spots();
}

bool building_type_is_monument(int32_t type)
{
    switch (type) {
        case BUILDING_PYRAMID:
        case BUILDING_MAUSOLEUM:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_SMALL_OBELISK:
        case BUILDING_LARGE_OBELISK:
        case BUILDING_SPHINX:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
        case BUILDING_LIGHTHOUSE:
            return 1;
        default:
            return 0;
    }
}

bool building_is_not_monument(int32_t building_id)
{
    switch (building_get(building_id)->type) {
        case BUILDING_PYRAMID:
        case BUILDING_SMALL_OBELISK:
        case BUILDING_SPHINX:
        case BUILDING_SUN_TEMPLE:
        case BUILDING_PYRAMID_ENTRY_TILE:
        case BUILDING_MORTUARY_TEMPLE:
        case BUILDING_CAUSEWAY:
        case BUILDING_VALLEY_TEMPLE:
        case BUILDING_MAUSOLEUM:
        case BUILDING_ALEXANDRIA_LIBRARY:
        case BUILDING_CAESAREUM:
        case BUILDING_LIGHTHOUSE:
        case BUILDING_SMALL_ROYAL_TOMB:
        case BUILDING_ABU_SIMBEL:
        case BUILDING_MEDIUM_ROYAL_TOMB:
        case BUILDING_LARGE_ROYAL_TOMB:
        case BUILDING_GRAND_ROYAL_TOMB:
            return 0;
        default:
            return 1;
    }
}

static void set_completion_phrases(int16_t *phrases, int32_t title)
{
    phrases[MONUMENT_COMPLETION_TITLE] = (int16_t) title;
    phrases[MONUMENT_COMPLETION_TEXT] = (int16_t) (title + 1);
}

static void post_completion_message(int16_t *phrases)
{
    city_message_use_popup = 1;
    city_message_post(MESSAGE_MONUMENT_COMPLETED, -1, 0, phrases, 0);
}

void monuments_check_completion(void)
{
    int16_t phrases[15] = { -1, -1, -1 };
    for (int32_t id = 1; id < MAX_BUILDINGS; id++) {
        building *b = building_get(id);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        switch (b->type) {
            case BUILDING_PYRAMID:
                if (pyramid_is_complete(id) && !b->data.pyramid.completion_shown) {
                    switch (scenario_pyramid_style) {
                        case PYRAMID_STYLE_BENT:
                            set_completion_phrases(phrases, MONUMENT_PHRASE_BENT_PYRAMID);
                            break;
                        case PYRAMID_STYLE_MUDBRICK:
                            set_completion_phrases(phrases, MONUMENT_PHRASE_MUDBRICK_PYRAMID);
                            break;
                        case PYRAMID_STYLE_STEPPED:
                            set_completion_phrases(phrases, MONUMENT_PHRASE_STEPPED_PYRAMID);
                            break;
                        case PYRAMID_STYLE_PYRAMID:
                            set_completion_phrases(phrases, MONUMENT_PHRASE_PYRAMID);
                            break;
                        case PYRAMID_STYLE_MASTABA:
                            set_completion_phrases(phrases, MONUMENT_PHRASE_MASTABA);
                            break;
                    }
                    b->data.pyramid.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                if (scenario_pyramid_style == PYRAMID_STYLE_MASTABA) {
                    id += mastaba_part_count[b->data.pyramid.size_variant] - 1;
                } else {
                    id += pyramid_part_count[b->data.pyramid.size_variant] - 1;
                }
                break;
            case BUILDING_SMALL_OBELISK:
            case BUILDING_LARGE_OBELISK:
                if (b->data.obelisk.progress == MONUMENT_PROGRESS_COMPLETE && !b->data.obelisk.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_OBELISK);
                    b->data.obelisk.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                break;
            case BUILDING_SPHINX: {
                city_player *player = city_player_current();
                if (player->sphinx_stage >= SPHINX_STAGE_COUNT && player->sphinx_completed == 0) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_SPHINX);
                    player->sphinx_completed = 1;
                    post_completion_message(phrases);
                    return;
                }
                break;
            }
            case BUILDING_SUN_TEMPLE:
                if (sun_temple_get_progress(id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.monument.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_SUN_TEMPLE);
                    b->data.monument.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += SUN_TEMPLE_PARTS - 1;
                break;
            case BUILDING_MAUSOLEUM:
                if (mausoleum_get_progress(id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.mausoleum.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_MAUSOLEUM);
                    b->data.mausoleum.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += MAUSOLEUM_PARTS - 1;
                break;
            case BUILDING_ALEXANDRIA_LIBRARY:
                if (library_get_progress(&library_object, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.monument.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_LIBRARY);
                    b->data.monument.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += LIBRARY_PARTS - 1;
                break;
            case BUILDING_CAESAREUM:
                if (caesareum_get_progress(&caesareum_object, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.monument.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_CAESAREUM);
                    b->data.monument.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += CAESAREUM_PARTS - 1;
                break;
            case BUILDING_LIGHTHOUSE:
                if (lighthouse_get_progress(&lighthouse_object, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.monument.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_LIGHTHOUSE);
                    b->data.monument.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += LIGHTHOUSE_PARTS - 1;
                break;
            case BUILDING_ABU_SIMBEL:
                if (abu_simbel_get_progress(&abu_simbel_object, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.abu_simbel.completion_shown) {
                    set_completion_phrases(phrases, MONUMENT_PHRASE_ABU_SIMBEL);
                    b->data.abu_simbel.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += ABU_SIMBEL_PARTS - 1;
                break;
            case BUILDING_SMALL_ROYAL_TOMB:
                if (royal_tomb_progress(BUILDING_SMALL_ROYAL_TOMB, id, 0) >= MONUMENT_PROGRESS_COMPLETE && (int16_t) b->created_sequence >= -1 &&
                    !b->data.royal_tomb.completion_shown) {
                    royal_tombs_update_all_images();
                    map_tiles_update_all();
                    set_completion_phrases(phrases, MONUMENT_PHRASE_SMALL_ROYAL_TOMB);
                    b->data.royal_tomb.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += SMALL_TOMB_PARTS - 1;
                break;
            case BUILDING_MEDIUM_ROYAL_TOMB:
                if (royal_tomb_progress(BUILDING_MEDIUM_ROYAL_TOMB, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.royal_tomb.completion_shown) {
                    royal_tombs_update_all_images();
                    map_tiles_update_all();
                    set_completion_phrases(phrases, MONUMENT_PHRASE_MEDIUM_ROYAL_TOMB);
                    b->data.royal_tomb.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += MEDIUM_TOMB_PARTS - 1;
                break;
            case BUILDING_LARGE_ROYAL_TOMB:
                if (royal_tomb_progress(BUILDING_LARGE_ROYAL_TOMB, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.royal_tomb.completion_shown) {
                    royal_tombs_update_all_images();
                    map_tiles_update_all();
                    set_completion_phrases(phrases, MONUMENT_PHRASE_LARGE_ROYAL_TOMB);
                    b->data.royal_tomb.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += LARGE_TOMB_PARTS - 1;
                break;
            case BUILDING_GRAND_ROYAL_TOMB:
                if (royal_tomb_progress(BUILDING_GRAND_ROYAL_TOMB, id, 0) >= MONUMENT_PROGRESS_COMPLETE && !b->data.royal_tomb.completion_shown) {
                    royal_tombs_update_all_images();
                    map_tiles_update_all();
                    set_completion_phrases(phrases, MONUMENT_PHRASE_GRAND_ROYAL_TOMB);
                    b->data.royal_tomb.completion_shown = 1;
                    post_completion_message(phrases);
                    return;
                }
                id += GRAND_TOMB_PARTS - 1;
                break;
        }
    }
}

static int32_t unplaced_need(int32_t def_id, int32_t resource)
{
    if ((uint32_t) def_id >= MONUMENT_DEFINITIONS) {
        return 0;
    }
    const monument_unplaced_need *need = &monument_unplaced_needs[def_id];
    if (need->resource != RESOURCE_NONE && resource == need->resource) {
        return need->amount;
    }
    return (resource == RESOURCE_LIMESTONE) ? need->limestone : 0;
}

int32_t monuments_unplaced_resource_need(int32_t resource)
{
    int32_t total = 0;
    for (int32_t slot = 0; slot < MONUMENT_SLOTS; slot++) {
        int32_t id = city_player_current()->monument_building_id[slot];
        building *b = id ? building_get(id) : 0;
        if (!b || b->state != BUILDING_STATE_IN_USE) {
            total += unplaced_need(scenario_data.monument_def_ids[slot], resource);
            continue;
        }
        int32_t type = b->type;
        if (type == BUILDING_PYRAMID) {
            pyramid_progress progress;
            pyramid_get_progress_info(id, &progress);
            int32_t main = progress.main_remaining;
            int32_t casing = progress.casing_remaining;
            switch (scenario_pyramid_style) {
                case PYRAMID_STYLE_BENT:
                case PYRAMID_STYLE_PYRAMID:
                    if (resource == RESOURCE_LIMESTONE) {
                        if (casing > 0) {
                            total += casing * MONUMENT_MATERIAL_PER_LOAD;
                        }
                    } else if (resource == RESOURCE_STONE && main > 0) {
                        total += main * MONUMENT_MATERIAL_PER_LOAD;
                    }
                    break;
                case PYRAMID_STYLE_MUDBRICK:
                    if (resource == RESOURCE_LIMESTONE) {
                        if (casing > 0) {
                            total += casing * MONUMENT_MATERIAL_PER_LOAD;
                        }
                    } else if (resource == RESOURCE_BRICKS && main > 0) {
                        total += main * MONUMENT_MATERIAL_PER_LOAD;
                    }
                    break;
                case PYRAMID_STYLE_STEPPED:
                    if (resource == RESOURCE_STONE && (main > 0 || casing > 0)) {
                        total += (main + casing) * MONUMENT_MATERIAL_PER_LOAD;
                    }
                    break;
                case PYRAMID_STYLE_MASTABA:
                    if (resource == RESOURCE_BRICKS && (main > 0 || casing > 0)) {
                        total += (main + casing) * MONUMENT_MATERIAL_PER_LOAD;
                    }
                    break;
            }
        } else if (type == BUILDING_SUN_TEMPLE) {
            int32_t progress[32];
            sun_temple_get_progress(id, progress);
            if (resource == RESOURCE_SANDSTONE) {
                total += progress[SUN_TEMPLE_PROGRESS_MATERIAL];
            }
        } else if (type == BUILDING_MAUSOLEUM) {
            mausoleum_info_progress progress;
            mausoleum_get_progress(id, &progress);
            if (resource == RESOURCE_SANDSTONE) {
                total += progress.material;
            }
        }
    }
    return total;
}

void monument_load_animation_scripts(void)
{
    if (scenario_data.animation_set <= MONUMENT_ERA_PYRAMIDS) {
        scenario_data.animation_set = MONUMENT_ERA_PYRAMIDS;
        scenario_pyramid_style = scenario_get_pyramid_style();
        const char *pyramid;
        switch (scenario_pyramid_style) {
            case PYRAMID_STYLE_BENT:
                pyramid = "BENT_PYRAMID";
                break;
            case PYRAMID_STYLE_MUDBRICK:
                pyramid = "MUDBRICK_PYRAMID";
                break;
            case PYRAMID_STYLE_STEPPED:
                pyramid = "STEPPED_PYRAMID";
                break;
            case PYRAMID_STYLE_MASTABA:
                pyramid = "MASTABA";
                break;
            default:
                pyramid = "PYRAMID";
                break;
        }
        anim_load_script(pyramid, MONUMENT_SCRIPT_SLOT_PYRAMID);
        anim_load_script("CaesareumDummy", MONUMENT_SCRIPT_SLOT_CAESAREUM);
        anim_load_script("LtHouseDummy", MONUMENT_SCRIPT_SLOT_LIGHTHOUSE);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_A);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_B);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_C);
        anim_load_script("LibExtraDummy", LIBRARY_SCRIPT_SLOT_EXTRA);
        anim_load_script("MausCarryoverDummy", MONUMENT_SCRIPT_SLOT_MAUSOLEUM);
        anim_load_script("TombThutDummy", MONUMENT_SCRIPT_SLOT_TOMB_THUT);
        anim_load_script("TombTutDummy", MONUMENT_SCRIPT_SLOT_TOMB_TUT);
        anim_load_script("TombSetiDummy", MONUMENT_SCRIPT_SLOT_TOMB_SETI);
        anim_load_script("TombRamDummy", MONUMENT_SCRIPT_SLOT_TOMB_RAM);
        anim_load_script("AbuSimbelDummy", MONUMENT_SCRIPT_SLOT_ABU_SIMBEL);
    } else if (scenario_data.animation_set == MONUMENT_ERA_VALLEY_OF_THE_KINGS) {
        anim_load_script("PyramidDummy", MONUMENT_SCRIPT_SLOT_PYRAMID);
        anim_load_script("CaesareumDummy", MONUMENT_SCRIPT_SLOT_CAESAREUM);
        anim_load_script("LtHouseDummy", MONUMENT_SCRIPT_SLOT_LIGHTHOUSE);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_A);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_B);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_C);
        anim_load_script("LibExtraDummy", LIBRARY_SCRIPT_SLOT_EXTRA);
        anim_load_script("MausCarryoverDummy", MONUMENT_SCRIPT_SLOT_MAUSOLEUM);
        anim_load_script("AbuSimbelDummy", MONUMENT_SCRIPT_SLOT_ABU_SIMBEL);
        int32_t mission = settings_data.mission_id;
        anim_load_script((mission >= MISSION_VALLEY_TUTANKHAMUN_TOMB && mission <= MISSION_VALLEY_RAMESSES_TOMB) ? "TombThutFin" : "TombThut", MONUMENT_SCRIPT_SLOT_TOMB_THUT);
        anim_load_script((mission == MISSION_VALLEY_SETI_TOMB || mission == MISSION_VALLEY_RAMESSES_TOMB) ? "TombTutFin" : "TombTut", MONUMENT_SCRIPT_SLOT_TOMB_TUT);
        anim_load_script((mission == MISSION_VALLEY_RAMESSES_TOMB) ? "TombSetiFin" : "TombSeti", MONUMENT_SCRIPT_SLOT_TOMB_SETI);
        anim_load_script("TombRam", MONUMENT_SCRIPT_SLOT_TOMB_RAM);
    } else if (scenario_data.animation_set == MONUMENT_ERA_ABU_SIMBEL) {
        anim_load_script("AbuSimbel", MONUMENT_SCRIPT_SLOT_ABU_SIMBEL);
        anim_load_script("PyramidDummy", MONUMENT_SCRIPT_SLOT_PYRAMID);
        anim_load_script("CaesareumDummy", MONUMENT_SCRIPT_SLOT_CAESAREUM);
        anim_load_script("LtHouseDummy", MONUMENT_SCRIPT_SLOT_LIGHTHOUSE);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_A);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_B);
        anim_load_script(s_sphinx1d.libmain_dummy, LIBRARY_SCRIPT_SLOT_C);
        anim_load_script("LibExtraDummy", LIBRARY_SCRIPT_SLOT_EXTRA);
        anim_load_script("MausCarryoverDummy", MONUMENT_SCRIPT_SLOT_MAUSOLEUM);
        anim_load_script("TombThutDummy", MONUMENT_SCRIPT_SLOT_TOMB_THUT);
        anim_load_script("TombSetiDummy", MONUMENT_SCRIPT_SLOT_TOMB_SETI);
        anim_load_script("TombTutDummy", MONUMENT_SCRIPT_SLOT_TOMB_TUT);
        anim_load_script("TombRamDummy", MONUMENT_SCRIPT_SLOT_TOMB_RAM);
    } else {
        anim_load_script("PyramidDummy", MONUMENT_SCRIPT_SLOT_PYRAMID);
        anim_load_script("TombThutDummy", MONUMENT_SCRIPT_SLOT_TOMB_THUT);
        anim_load_script("TombTutDummy", MONUMENT_SCRIPT_SLOT_TOMB_TUT);
        anim_load_script("TombSetiDummy", MONUMENT_SCRIPT_SLOT_TOMB_SETI);
        anim_load_script("TombRamDummy", MONUMENT_SCRIPT_SLOT_TOMB_RAM);
        anim_load_script("Caesareum1", MONUMENT_SCRIPT_SLOT_CAESAREUM);
        anim_load_script(lighthouse_script_names[LIGHTHOUSE_SCRIPTS - 1], MONUMENT_SCRIPT_SLOT_LIGHTHOUSE);
        anim_load_script("LibMaina1", LIBRARY_SCRIPT_SLOT_A);
        anim_load_script("LibMainb1", LIBRARY_SCRIPT_SLOT_B);
        anim_load_script("LibMainc1", LIBRARY_SCRIPT_SLOT_C);
        anim_load_script("LibExtra", LIBRARY_SCRIPT_SLOT_EXTRA);
        anim_load_script("MausCarryover", MONUMENT_SCRIPT_SLOT_MAUSOLEUM);
        anim_load_script("AbuSimbelDummy", MONUMENT_SCRIPT_SLOT_ABU_SIMBEL);
    }
}
