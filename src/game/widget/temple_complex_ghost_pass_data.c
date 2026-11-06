// Built-in draw order of the temple complex ghost's decoration tiles; config/temple_complex_ghost_passes.json can
// override it.

#include "widget/construction_ghost_temple_complex.h"

temple_complex_ghost_pass temple_complex_ghost_passes[TEMPLE_COMPLEX_GHOST_VIEWS] = {
    [0] = {
        .image_field = { 1, 4, 3, 2 },
        .first_rows_from = { 0, 5, 2, 1 },
        .first_rows_to = { 4, 6, 6, 0 },
        .second_rows_from = { 5, 0, 1, 2 },
        .second_rows_to = { 6, 4, 0, 6 },
    },
    [1] = {
        .image_field = { 3, 2, 1, 4 },
        .first_rows_from = { 2, 0, 0, 2 },
        .first_rows_to = { 6, 4, 4, 6 },
        .second_rows_from = { 1, 5, 5, 1 },
        .second_rows_to = { 0, 6, 6, 0 },
    },
    [2] = {
        .image_field = { 1, 4, 3, 2 },
        .first_rows_from = { 1, 2, 5, 0 },
        .first_rows_to = { 0, 6, 6, 4 },
        .second_rows_from = { 2, 1, 4, 5 },
        .second_rows_to = { 6, 0, 0, 6 },
    },
    [3] = {
        .image_field = { 3, 2, 1, 4 },
        .first_rows_from = { 5, 1, 1, 5 },
        .first_rows_to = { 6, 0, 0, 6 },
        .second_rows_from = { 4, 2, 2, 4 },
        .second_rows_to = { 0, 6, 6, 0 },
    },
};
