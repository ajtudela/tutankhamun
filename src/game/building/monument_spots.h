// The work spots of the obelisks, sphinx, sun temple and monument objects: spot animation, work, stages and
// images, the completion phrases and the material needed by monuments not yet placed.

#ifndef BUILDING_MONUMENT_SPOTS_H
#define BUILDING_MONUMENT_SPOTS_H

#include "building/building_create.h"
#include "building/construction_load.h"
#include "building/monument.h"
#include "building/monument_data.h"
#include "building/monument_internal.h"
#include "city/resource_type.h"
#include "figure/figure_type.h"
#include "game/undo_build.h"
#include "graphics/image_collection.h"
#include "graphics/image_draw.h"
#include "map/building_tiles.h"
#include <stdbool.h>
#include <stdint.h>

enum {
    SUN_TEMPLE_WORK_SPOTS = 13,
    SPHINX_WORK_SPOTS = 22,
    OBELISK_SMALL_WORK_SPOTS = 10,
    OBELISK_LARGE_WORK_SPOTS = 18,
    LIBRARY_WORK_SPOTS = 27,
    ABU_SIMBEL_WORK_SPOTS = 56,
    CAESAREUM_WORK_SPOTS = 27,
    LIGHTHOUSE_WORK_SPOTS = 15,
    ROYAL_TOMB_SPOTS_PER_TABLE = 3,
};

enum monument_script_slot {
    MONUMENT_SCRIPT_SLOT_STAGE_A = 5,
    MONUMENT_SCRIPT_SLOT_STAGE_B = 6,
    MONUMENT_SCRIPT_SLOT_STAGE_C = 7,
    MONUMENT_SCRIPT_SLOT_SPHINX_1D = 8,
    MONUMENT_SCRIPT_SLOT_EXTRA = 9,
    MONUMENT_SCRIPT_SLOT_PYRAMID = 17,
    MONUMENT_SCRIPT_SLOT_CAESAREUM = 18,
    MONUMENT_SCRIPT_SLOT_LIGHTHOUSE = 19,
    MONUMENT_SCRIPT_SLOT_MAUSOLEUM = 24,
    MONUMENT_SCRIPT_SLOT_TOMB_THUT = 25,
    MONUMENT_SCRIPT_SLOT_ABU_SIMBEL = 26,
    MONUMENT_SCRIPT_SLOT_TOMB_TUT = 27,
    MONUMENT_SCRIPT_SLOT_TOMB_SETI = 28,
    MONUMENT_SCRIPT_SLOT_TOMB_RAM = 29,
};

enum valley_tomb_mission {
    MISSION_VALLEY_TUTANKHAMUN_TOMB = 39,
    MISSION_VALLEY_SETI_TOMB = 40,
    MISSION_VALLEY_RAMESSES_TOMB = 41,
};

enum monument_completion_phrase {
    MONUMENT_COMPLETION_TEXT = 0,
    MONUMENT_COMPLETION_TITLE = 1,
    MONUMENT_PHRASE_PYRAMID = 550,
    MONUMENT_PHRASE_STEPPED_PYRAMID = 552,
    MONUMENT_PHRASE_BENT_PYRAMID = 554,
    MONUMENT_PHRASE_MUDBRICK_PYRAMID = 556,
    MONUMENT_PHRASE_MASTABA = 558,
    MONUMENT_PHRASE_SPHINX = 560,
    MONUMENT_PHRASE_OBELISK = 562,
    MONUMENT_PHRASE_SUN_TEMPLE = 564,
    MONUMENT_PHRASE_LIBRARY = 566,
    MONUMENT_PHRASE_ABU_SIMBEL = 568,
    MONUMENT_PHRASE_CAESAREUM = 570,
    MONUMENT_PHRASE_LIGHTHOUSE = 572,
    MONUMENT_PHRASE_MAUSOLEUM = 574,
    MONUMENT_PHRASE_SMALL_ROYAL_TOMB = 576,
    MONUMENT_PHRASE_MEDIUM_ROYAL_TOMB = 578,
    MONUMENT_PHRASE_LARGE_ROYAL_TOMB = 580,
    MONUMENT_PHRASE_GRAND_ROYAL_TOMB = 582,
};

typedef struct {
    int32_t resource;
    int32_t amount;
    int32_t limestone;
} monument_unplaced_need;

extern monument_unplaced_need monument_unplaced_needs[MONUMENT_DEFINITIONS];

void monument_spot_start_anim(monument_spot *spot, int32_t mode);
void monument_spot_attach_figure(monument_spot *spot, int32_t figure_id);
void monument_spot_animate(monument_spot *spot);
void monument_spot_get_draw_info(monument_spot *spot, int32_t *x, int32_t *y, int32_t *image);

static inline char anim_load_script(const char *name, int32_t slot)
{
    return (char) image_collection_load_file(name, slot);
}

static inline void monument_draw_image(int32_t image_id, int32_t x, int32_t y)
{
    image_draw(image_id, x, y);
}

void sphinx_animate_spots(void);
void sun_temple_animate_spots(void);
void obelisk_animate_spots(void);
void sphinx_check_stage_done(void);
void sun_temple_check_stage_done(int32_t building_id);
void sun_temple_figure_work_tick_spot(int32_t building_id, int32_t figure_id, uint8_t next_action);
int32_t sun_temple_has_free_spot(int32_t building_id, int32_t figure_id);
void sun_temple_count_assigned(int32_t figure_type, int32_t *count);
void sphinx_advance_stage(void);
void sphinx_load_stage_scripts(void);
void obelisk_reset_work_spots(int32_t building_id);
void obelisk_update_image(int32_t building_id, int32_t x, int32_t y);
void obelisk_load_script_for_slot(int32_t slot);
int32_t obelisk_find_incomplete(void);
int32_t obelisk_get_current_stage(int32_t building_id);
int32_t obelisk_advance_progress(int32_t building_id);
void sun_temple_update_image(int32_t building_id);
int32_t sun_temple_part_has_active_worker(int32_t building_id);
int32_t sun_temple_check_work_for_figure(int32_t building_id, int32_t figure_id, bool assign);
int32_t sun_temple_get_progress(int32_t building_id, int32_t *out);
void sun_temple_advance_stage(int32_t building_id);
void caesareum_animate_spots(void);
void royal_tomb_animate_spots(void);
void library_animate_spots(void);
void abu_simbel_animate_spots(void);
void lighthouse_animate_spots(void);
void monument_spot_get_draw_info_dir(monument_spot *spot, int32_t *x, int32_t *y, int32_t *image, int32_t direction);

bool building_type_is_monument(int32_t type);
int32_t monuments_unplaced_resource_need(int32_t resource);
void monuments_animate_all_spots(void);
void monuments_check_completion(void);
void monument_load_animation_scripts(void);

#endif
