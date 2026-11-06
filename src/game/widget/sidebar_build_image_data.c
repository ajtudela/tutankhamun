// Built-in build menu pictures of the sidebar: per build menu group its picture's offset from the build image
// base (other groups show the first picture). config/sidebar_build_images.json can override them.

#include "widget/sidebar_buttons.h"

sidebar_build_image sidebar_build_images[SIDEBAR_BUILD_GROUPS] = {
    [0] = { 0 },
    [1] = { 1 },
    [2] = { 9 },
    [3] = { 5 },
    [4] = { 10 },
    [5] = { 4 },
    [6] = { 7 },
    [7] = { 11 },
    [8] = { 3 },
    [9] = { 8 },
    [10] = { 6 },
    [11] = { 12 },
    [12] = { 2 },
};
