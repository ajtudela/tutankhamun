// The pixel offsets of the monument parts for the current tile size, computed once at start-up: the scripted
// monuments rotate their part descriptors, the sun temple's and mausoleum's get their grid delta and offsets in
// the four views.

#include "building/monument_part_offsets.h"
#include "building/mausoleum.h"
#include "building/monument_object.h"
#include "building/royal_tomb.h"
#include "building/sun_temple.h"
#include "map/grid.h"
#include "widget/widget_data.h"
#include <stdint.h>

void abu_simbel_rotate_part_descs(abu_simbel_object_t *self)
{
    monument_part_desc_rotate_n(self->parts, ABU_SIMBEL_PARTS);
}

void library_rotate_part_descs(library_object_t *self)
{
    monument_part_desc_rotate_n(self->parts, LIBRARY_PARTS);
}

void caesareum_rotate_part_descs(caesareum_object_t *self)
{
    monument_part_desc_rotate_n(self->parts, CAESAREUM_PARTS);
}

void lighthouse_rotate_parts(lighthouse_object_t *self)
{
    monument_part_desc_rotate_n(self->parts, LIGHTHOUSE_PARTS);
}

void small_royal_tomb_rotate_part_descs(tomb_object *self)
{
    monument_part_desc_rotate_n(royal_tomb_record_of(self)->small.descs, SMALL_TOMB_PARTS);
}

void medium_royal_tomb_rotate_parts(tomb_object *self)
{
    monument_part_desc_rotate_n(royal_tomb_record_of(self)->medium.descs, MEDIUM_TOMB_PARTS);
}

void large_royal_tomb_rotate_parts(tomb_object *self)
{
    monument_part_desc_rotate_n(royal_tomb_record_of(self)->large.descs, LARGE_TOMB_PARTS);
}

void grand_royal_tomb_rotate_parts(tomb_object *self)
{
    monument_part_desc_rotate_n(royal_tomb_record_of(self)->grand.descs, GRAND_TOMB_PARTS);
}

void sun_temple_init_part_offsets(void)
{
    int32_t step_y = city_view.tile_half_height;
    int32_t step_x = city_view.tile_half_width + 1;
    for (monument_part_desc *d = sun_temple_part_descs; d < sun_temple_part_descs + SUN_TEMPLE_PARTS; d++) {
        int32_t x = d->dx;
        int32_t y = d->dy;
        d->grid_delta = x + y * GRID_SIZE;
        d->view_x[0] = (x - y) * step_x;
        d->view_x[1] = (x + y) * step_x;
        d->view_x[2] = (y - x) * step_x;
        d->view_x[3] = -((x + y) * step_x);
        d->view_y[0] = (x + y) * step_y;
        d->view_y[1] = (y - x) * step_y;
        d->view_y[2] = -((x + y) * step_y);
        d->view_y[3] = (x - y) * step_y;
    }
}

void mausoleum_init_part_offsets(void)
{
    int32_t step_y = city_view.tile_half_height;
    int32_t step_x = city_view.tile_half_width + 1;
    monument_part_desc *d = mausoleum_part_descs;
    for (int32_t part = 0; d < mausoleum_part_descs + MAUSOLEUM_PARTS; part++, d++) {
        monument_part_desc r;
        mausoleum_get_part_desc(&r, part, 0, 0);
        d->grid_delta = r.dx + r.dy * GRID_SIZE;
        d->view_x[0] = (r.dx - r.dy) * step_x;
        d->view_y[0] = (r.dy + r.dx) * step_y;
        mausoleum_get_part_desc(&r, part, 2, 2);
        d->view_x[1] = (r.dy + r.dx) * step_x;
        d->view_y[1] = (r.dy - r.dx) * step_y;
        mausoleum_get_part_desc(&r, part, 4, 4);
        d->view_x[2] = (r.dy - r.dx) * step_x;
        d->view_y[2] = -((r.dy + r.dx) * step_y);
        mausoleum_get_part_desc(&r, part, 6, 6);
        d->view_x[3] = -((r.dy + r.dx) * step_x);
        d->view_y[3] = (r.dx - r.dy) * step_y;
    }
}
