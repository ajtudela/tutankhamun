// The part descriptors of the scripted monument objects and the royal tomb constructors, which fill each tomb's
// per-part arrays from its part table.
//
// A tomb draws one rand() % 5 per random closed image variant, by increasing part index, when it is built.

#include "building/monument_static.h"
#include "building/abu_simbel.h"
#include "building/building_data.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/monument_data.h"
#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include "core/crt_compat.h"
#include <stdint.h>
#include <string.h>

typedef struct {
    int32_t size;
    const tomb_object_vtable *vtable;
    int32_t last_part;
} tomb_def;

static const tomb_def small_tomb = { ROYAL_TOMB_SMALL, &small_royal_tomb_vtable, SMALL_TOMB_PARTS - 1 };
static const tomb_def medium_tomb = { ROYAL_TOMB_MEDIUM, &medium_royal_tomb_vtable, MEDIUM_TOMB_CHAMBERS };
static const tomb_def large_tomb = { ROYAL_TOMB_LARGE, &large_royal_tomb_vtable, LARGE_TOMB_PARTS - 1 };
static const tomb_def grand_tomb = { ROYAL_TOMB_GRAND, &grand_royal_tomb_vtable, GRAND_TOMB_CHAMBERS };

static void tomb_construct(tomb_object *self, const tomb_def *t)
{
    const royal_tomb_part_table *table = &royal_tomb_part_tables[t->size];
    int32_t n = table->count;
    royal_tomb_parts a = royal_tomb_parts_of(self, t->size);
    royal_tomb_object_base_init(self);
    self->vtable = t->vtable;
    self->last_part = t->last_part;
    for (int32_t i = 0; i < n; i++) {
        const royal_tomb_part_init *p = &table->parts[i];
        a.descs[i] = (monument_part_desc) { p->dx, p->dy, p->size, i };
    }
    for (int32_t i = 0; i < n; i++) {
        const royal_tomb_part_init *p = &table->parts[i];
        a.anim_key[i] = p->anim_key;
        a.chamber_kind[i] = p->chamber_kind;
        int32_t *neighbours = a.neighbours[i];
        for (int32_t k = 0; k < TOMB_NEIGHBOURS; k++) {
            neighbours[k] = (k < ROYAL_TOMB_PART_NEIGHBOURS_LISTED) ? p->neighbours[k] : TOMB_NO_NEIGHBOUR;
        }
        for (int32_t stage = 0; stage < TOMB_WORK_STAGES; stage++) {
            a.work[i][stage] = p->work;
        }
        a.completed_image[i] = p->completed_image;
        if (p->closed_image_variant == ROYAL_TOMB_CLOSED_VARIANT_RANDOM) {
            a.closed_image_variant[i] = (uint8_t) (crt_rand() % ROYAL_TOMB_CLOSED_VARIANTS);
        } else {
            a.closed_image_variant[i] = (uint8_t) p->closed_image_variant;
        }
    }
}

void royal_tomb_object_base_init(tomb_object *self)
{
    self->base_part = (monument_part_desc) { 0, 0, 1, 7 };
    self->base_scripts[0] = sun_temple_scripts[0];
    self->base_scripts[1] = sun_temple_scripts[1];
    self->base_scripts[2] = sun_temple_scripts[2];
    self->frame_count = 1;
    self->anim_frame = 0;
}

void small_royal_tomb_object_init(tomb_object *self)
{
    tomb_construct(self, &small_tomb);
}

void medium_royal_tomb_object_init(tomb_object *self)
{
    tomb_construct(self, &medium_tomb);
}

void large_royal_tomb_object_init(tomb_object *self)
{
    tomb_construct(self, &large_tomb);
}

void grand_royal_tomb_object_init(tomb_object *self)
{
    tomb_construct(self, &grand_tomb);
}
