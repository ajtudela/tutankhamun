// The built-in part images of the royal tombs: per tomb size the sealing and entrance image groups and per
// chamber kind its image group. config/royal_tomb_images.json can override them.

#include "building/royal_tomb.h"
#include "core/config.h"

static royal_tomb_chamber_image small_chambers[] = {
    { 0, 791 },
    { 1, 793 },
    { 2, 795 },
    { 3, 797 },
    { 6, 799 },
    { 15, 801 },
    { 16, 803 },
    { 29, 810 },
};

static royal_tomb_chamber_image medium_chambers[] = {
    { 0, 828 },
    { 1, 830 },
    { 2, 832 },
    { 9, 834 },
    { 10, 836 },
    { 19, 838 },
    { 20, 840 },
    { 21, 842 },
    { 29, 849 },
};

static royal_tomb_chamber_image large_chambers[] = {
    { 0, 851 },
    { 1, 853 },
    { 2, 855 },
    { 4, 857 },
    { 5, 859 },
    { 6, 861 },
    { 7, 863 },
    { 8, 865 },
    { 12, 867 },
    { 22, 869 },
    { 23, 871 },
    { 29, 878 },
};

static royal_tomb_chamber_image grand_chambers[] = {
    { 0, 880 },
    { 1, 882 },
    { 2, 884 },
    { 4, 886 },
    { 6, 888 },
    { 7, 890 },
    { 8, 892 },
    { 11, 894 },
    { 12, 896 },
    { 13, 898 },
    { 14, 900 },
    { 24, 902 },
    { 25, 904 },
    { 26, 906 },
    { 29, 913 },
};

royal_tomb_image_set royal_tomb_image_sets[ROYAL_TOMB_SIZES] = {
    [ROYAL_TOMB_SMALL] = { 0, 1, 805, { 807, 808, 809 }, small_chambers, CONFIG_COUNT(small_chambers) },
    [ROYAL_TOMB_MEDIUM] = { 1, 0, 844, { 846, 847, 848 }, medium_chambers, CONFIG_COUNT(medium_chambers) },
    [ROYAL_TOMB_LARGE] = { 1, 0, 873, { 875, 876, 877 }, large_chambers, CONFIG_COUNT(large_chambers) },
    [ROYAL_TOMB_GRAND] = { 1, 0, 908, { 910, 911, 912 }, grand_chambers, CONFIG_COUNT(grand_chambers) },
};
