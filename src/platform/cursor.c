// The play build's own mouse cursors: an arrow, a shovel and a sword as pixel art at the scales 1, 1.5 and 2, made
// into SDL colour cursors at start-up.
//
// Taken from Akhenaten (https://github.com/dalerank/Akhenaten, compared at commit
// e42f724ebd9d4b83de941300e7929a04e6d82adb, 2026-11-04): the pixel art with its sizes and hot spots from
// src/input/cursor.cpp (rows copied unchanged; Akhenaten has them from Julius' src/input/cursor.c,
// https://github.com/bvschaik/julius, AGPL-3.0), the colours from src/platform/cursor.h and the creation from
// src/platform/cursor.cpp. Copyright (C) the Akhenaten authors and the Julius authors. Licence: GNU Affero General
// Public License v3.0 (LICENSES/AGPL-3.0.txt). The upstream files have no copyright line of their own.

#include "platform/cursor.h"

#include "SDL.h"

typedef struct {
    int32_t hotspot_x;
    int32_t hotspot_y;
    int32_t width;
    int32_t height;
    const char *pixels;
} cursor_art;

static const char arrow_1_pixels[] =
    "#            "
    "##           "
    "#'#          "
    "#''#         "
    "#'''#        "
    "#''''#       "
    "#'''''#      "
    "#''''''#     "
    "#'''''''#    "
    "#''''''''#   "
    "#'''''''''#  "
    "#''''''''''# "
    "#''''''######"
    "#'''#''#     "
    "#''# #''#    "
    "#'#  #''#    "
    "##    #''#   "
    "#     #''#   "
    "       #''#  "
    "       #''#  "
    "        ##   ";

static const char arrow_1_5_pixels[] =
    "#                 "
    "##                "
    "#&#               "
    "#'&#              "
    "#''&#             "
    "#'''&#            "
    "#''''&#           "
    "#'''''&#          "
    "#''''''&#         "
    "#'''''''&#        "
    "#''''''''&#       "
    "#'''''''''&#      "
    "#''''''''''&#     "
    "#'''''''''''&#    "
    "#''''''''''''&#   "
    "#'''''''''''''&#  "
    "#''''''''''''''&# "
    "#'''''''''''''''$#"
    "#'''''''&#########"
    "#''''$%''$        "
    "#'''$##&'&#       "
    "#''$#  $''%#      "
    "#&#    #&''#      "
    "##      #''%#     "
    "#       #%''#     "
    "         #''&#    "
    "         #%''$    "
    "          #&'&#   "
    "           $'&#   "
    "           ###    ";

static const char arrow_2_pixels[] =
    "#                       "
    "##                      "
    "#&#                     "
    "#'&#                    "
    "#''&#                   "
    "#'''&#                  "
    "#''''&#                 "
    "#'''''&#                "
    "#''''''&#               "
    "#'''''''&#              "
    "#''''''''&#             "
    "#'''''''''&#            "
    "#''''''''''&#           "
    "#'''''''''''&#          "
    "#''''''''''''&#         "
    "#'''''''''''''&#        "
    "#''''''''''''''&#       "
    "#'''''''''''''''&#      "
    "#''''''''''''''''&#     "
    "#'''''''''''''''''&#    "
    "#''''''''''''''''''&#   "
    "#'''''''''''''''''''&#  "
    "#''''''''''''''''''''&# "
    "#'''''''''''''''''''''&#"
    "#''''''''''&############"
    "#''''''&&'''$           "
    "#'''''&##'''&#          "
    "#''''&# #%'''%#         "
    "#''''#   #''''#         "
    "#'''#    #%'''%#        "
    "#''#      #&'''#        "
    "#'$#       $'''&#       "
    "#$#        #&'''$       "
    "##          #'''&#      "
    "#           #%'''%#     "
    "             #''''#     "
    "             #%'''%#    "
    "              #&'''#    "
    "               $'&$#    "
    "               ###      ";

static const char shovel_1_pixels[] =
    "                       ##   "
    "                      ####  "
    "                     ##'### "
    "                     #'''###"
    "                     ##'''##"
    "                    ####'## "
    "                   ##'####  "
    "                  ##'##     "
    "                 ##'##      "
    "                ##'##       "
    "               ##'##        "
    "              ##'##         "
    "             ##'##          "
    "       #    ##'##           "
    "      #### ##'##            "
    "     #''####'##             "
    "    #''''##'##              "
    "   #''''##'##               "
    "  #''''#'#####              "
    " #''''#'''#'##              "
    " #'''#'''#'''##             "
    "#'''''''#''''#              "
    "#''''''#''''#               "
    "#''''''''''#                "
    "#'''''''''#                 "
    "#''''''''#                  "
    " #'''''##                   "
    "  #####                     ";

static const char shovel_1_5_pixels[] =
    "                                   ####     "
    "                                 #######    "
    "                                ##%&#####   "
    "                               ##&''$#####  "
    "                               #$'''''$#### "
    "                               #$''''''$####"
    "                               ##&''''''%## "
    "                              ####&''''$##  "
    "                             ##$''##&&&$#   "
    "                            ##$'''$#####    "
    "                           ##$'''$##        "
    "                          ##$'''$##         "
    "                         ##$'''$##          "
    "                        ##$'''$##           "
    "                       ##$'''$##            "
    "                      ##$'''$##             "
    "                     ##$'''$##              "
    "                    ##$'''$##               "
    "         ###       ##$'''$##                "
    "        ##$##     ##$'''$##                 "
    "       ##%'%##   ##$'''$##                  "
    "      ##%'''%## ##$'''$##                   "
    "     ##%'''''%##%''''$##                    "
    "    ##%''''''%#$''''$##                     "
    "   ##%''''''%#$''''$##                      "
    "  ##%''''''%#$#%''$##                       "
    "  #$''''''%#%'%#$###                        "
    "  #&'''''%#%'''$#%%##                       "
    " ##'''''%#%'''%#%''%##                      "
    " #$''''%#%'''%#%''''%##                     "
    " #&''''%%'''%#%''''''$#                     "
    " #&''''''''%#%''''''%##                     "
    "##''''''''%#%''''''%##                      "
    "#$''''''''%%''''''%##                       "
    "#$'''''''''''''''%##                        "
    "#$''''''''''''''%##                         "
    "#$'''''''''''''%##                          "
    "##'''''''''''&$##                           "
    " #%''''''''&$###                            "
    " ##%&''&%$###                               "
    "  #########                                 ";

static const char shovel_2_pixels[] =
    "                                               ##         "
    "                                             ######       "
    "                                            ###&#####     "
    "                                           ##$''&#####    "
    "                                          ##$''''&#####   "
    "                                         ##$'''''''$####  "
    "                                         ##$''''''''$#### "
    "                                         ##$'''''''''$####"
    "                                         ##$''''''''''$###"
    "                                         ###%'''''''''$## "
    "                                        ##%%#%''''''&##   "
    "                                       ##&''%#%''''&##    "
    "                                      ##&''''%#$$$$##     "
    "                                     ##&''''%#######      "
    "                                    ##&''''&###           "
    "                                   ##&''''&##             "
    "                                  ##&''''&##              "
    "                                 ##&''''&##               "
    "                                ##&''''&##                "
    "                               ##&''''&##                 "
    "                              ##&''''&##                  "
    "                             ##&''''&##                   "
    "                            ##&''''&##                    "
    "                           ##&''''&##                     "
    "             ##           ##&''''&##                      "
    "            ####         ##&''''&##                       "
    "           ##%%##       ##&''''&##                        "
    "          ##%''%##     ##&''''&##                         "
    "         ##%''''%##   ##&''''&##                          "
    "        ##%''''''%## ##%''''&##                           "
    "       ##%''''''''%##%'''''&##                            "
    "      ##%'''''''''%#$'''''&##                             "
    "     ##%'''''''''%#%'''''&##                              "
    "    ##%'''''''''%##%''''%##                               "
    "   ##%'''''''''%#%%#%''$##                                "
    "   #$'''''''''%#%''%#%%###                                "
    "   #&''''''''%#%''''%##%%##                               "
    "  ##''''''''%#%'''''%#%''%##                              "
    "  #%'''''''%#%'''''%#%''''%##                             "
    "  #&''''''%#%'''''%#%''''''%##                            "
    "  #''''''&#%'''''%#%''''''''%##                           "
    " #$'''''''&'''''%#%'''''''''%##                           "
    " #%''''''''''''%#%'''''''''%##                            "
    " #&'''''''''''%#%'''''''''%##                             "
    " #&''''''''''&#%'''''''''%##                              "
    " #''''''''''''&'''''''''%##                               "
    " #'''''''''''''''''''''%##                                "
    "##''''''''''''''''''''%##                                 "
    " #'''''''''''''''''''%##                                  "
    " #&'''''''''''''''''%##                                   "
    " #&'''''''''''''''&$##                                    "
    " #$'''''''''''''&$###                                     "
    "  #%'''''''''&%###                                        "
    "   #$%&&&&%$####                                          "
    "    #########                                             ";

static const char sword_1_pixels[] =
    "#####                 "
    "#'''##                "
    "#''''##               "
    "#'''''##              "
    "##'''''##             "
    " ##'''''##            "
    "  ##'''''##           "
    "   ##'''''##          "
    "    ##'''''##         "
    "     ##'''''##        "
    "      ##'''''##       "
    "       ##'''''##      "
    "        ##'''''#####  "
    "         ##'''''#''#  "
    "          ##'''#'''#  "
    "           ##'#'''##  "
    "            ##'''###  "
    "            #'''##'###"
    "            #''##''''#"
    "            ######'''#"
    "                 #''##"
    "                 #### ";

static const char sword_1_5_pixels[] =
    "######                            "
    "#%&&&##                           "
    "#&'''&##                          "
    "#&''''&##                         "
    "#&'''''&##                        "
    "##&'''''&##                       "
    " ##&'''''&##                      "
    "  ##&'''''&##                     "
    "   ##&'''''&##                    "
    "    ##&'''''&##                   "
    "     ##&'''''&##                  "
    "      ##&'''''&##                 "
    "       ##&'''''&##                "
    "        ##&'''''&##               "
    "         ##&'''''&##              "
    "          ##&'''''&##             "
    "           ##&'''''&##            "
    "            ##&'''''&##           "
    "             ##&'''''&##          "
    "              ##&'''''&########   "
    "               ##&''''''&##&&$#   "
    "                ##&''''&##&''$#   "
    "                 ##&''&##&''&##   "
    "                  ##%&##&''&###   "
    "                   #&##&''&####   "
    "                   ###&''&#####   "
    "                   ##&''&###%$### "
    "                   #&''&###%'%$$##"
    "                   #&'&###%'''''$#"
    "                   #$$####$%''''$#"
    "                   ########$'''&##"
    "                          #$''&#  "
    "                          ##$$##  "
    "                           ####   ";

static const char sword_2_pixels[] =
    "########                                      "
    "#%%%%%%##                                     "
    "#%'''''&##                                    "
    "#%''''''&##                                   "
    "#%'''''''%##                                  "
    "#%''''''''%##                                 "
    "#%'''''''''%##                                "
    "##%'''''''''%##                               "
    " ##%'''''''''%##                              "
    "  ##%'''''''''%##                             "
    "   ##%'''''''''%##                            "
    "    ##%'''''''''%##                           "
    "     ##%'''''''''%##                          "
    "      ##%'''''''''%##                         "
    "       ##%'''''''''%##                        "
    "        ##%'''''''''%##                       "
    "         ##%'''''''''%##                      "
    "          ##%'''''''''%##                     "
    "           ##%'''''''''%##                    "
    "            ##%'''''''''%##                   "
    "             ##%'''''''''%##                  "
    "              ##%'''''''''%##                 "
    "               ##%'''''''''%##                "
    "                ##%'''''''''%##               "
    "                 ##%'''''''''%##              "
    "                  ##%'''''''''%##########     "
    "                   ##%'''''''''%$$##$$$##     "
    "                    ##%''''''''''%#%'''$#     "
    "                     ##%''''''''%#%''''$#     "
    "                      ##%''''''%#%'''''$#     "
    "                       ##%''''%#%'''''%##     "
    "                        ##%''%#%'''''%###     "
    "                         #$'%#%'''''%####     "
    "                         #$%#%'''''%#####     "
    "                         ###%'''''%###%$#     "
    "                         ##%'''''%###%'$####  "
    "                         #$'''''%###%''%$$$## "
    "                         #$''''%###%'''''''$##"
    "                         #$'''%###%''''''''$##"
    "                         ##$$$####$$%'''''''$#"
    "                         ###########$''''$####"
    "                                   #$'''$&#   "
    "                                    #$''$#    "
    "                                     #####    ";

static const cursor_art cursor_arts[CURSOR_KINDS][CURSOR_SCALES] = {
    [CURSOR_KIND_ARROW] = {
        [CURSOR_SCALE_1] = { 0, 0, 13, 21, arrow_1_pixels },
        [CURSOR_SCALE_1_5] = { 0, 0, 18, 30, arrow_1_5_pixels },
        [CURSOR_SCALE_2] = { 0, 0, 24, 40, arrow_2_pixels },
    },
    [CURSOR_KIND_SHOVEL] = {
        [CURSOR_SCALE_1] = { 1, 26, 28, 28, shovel_1_pixels },
        [CURSOR_SCALE_1_5] = { 2, 39, 44, 41, shovel_1_5_pixels },
        [CURSOR_SCALE_2] = { 3, 52, 58, 55, shovel_2_pixels },
    },
    [CURSOR_KIND_SWORD] = {
        [CURSOR_SCALE_1] = { 0, 0, 22, 22, sword_1_pixels },
        [CURSOR_SCALE_1_5] = { 0, 0, 34, 34, sword_1_5_pixels },
        [CURSOR_SCALE_2] = { 0, 0, 46, 44, sword_2_pixels },
    },
};

typedef struct {
    char symbol;
    uint32_t argb;
} cursor_color;

static const cursor_color cursor_colors[] = {
    { ' ', 0x00000000 },
    { '#', 0xFF000000 },
    { '$', 0xFF3F3F3F },
    { '%', 0xFF737373 },
    { '&', 0xFFB3B3B3 },
    { '\'', 0xFFFFFFFF },
};

static SDL_Cursor *cursors[CURSOR_KINDS];

static uint32_t cursor_color_of(char symbol)
{
    for (size_t i = 0; i < sizeof(cursor_colors) / sizeof(cursor_colors[0]); i++) {
        if (cursor_colors[i].symbol == symbol) {
            return cursor_colors[i].argb;
        }
    }
    return 0;
}

static SDL_Surface *cursor_surface_create(const cursor_art *art)
{
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, art->width, art->height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surface) {
        return 0;
    }
    for (int32_t y = 0; y < art->height; y++) {
        uint32_t *row = (uint32_t *) ((uint8_t *) surface->pixels + (size_t) y * (size_t) surface->pitch);
        for (int32_t x = 0; x < art->width; x++) {
            row[x] = cursor_color_of(art->pixels[y * art->width + x]);
        }
    }
    return surface;
}

enum cursor_scale cursor_scale_for(int32_t scale_percent)
{
    if (scale_percent <= 100) {
        return CURSOR_SCALE_1;
    }
    if (scale_percent <= 150) {
        return CURSOR_SCALE_1_5;
    }
    return CURSOR_SCALE_2;
}

int32_t platform_cursors_create(int32_t scale_percent)
{
    enum cursor_scale scale = cursor_scale_for(scale_percent);
    int32_t created = 0;
    for (int32_t kind = 0; kind < CURSOR_KINDS; kind++) {
        const cursor_art *art = &cursor_arts[kind][scale];
        SDL_Surface *surface = cursor_surface_create(art);
        if (surface) {
            cursors[kind] = SDL_CreateColorCursor(surface, art->hotspot_x, art->hotspot_y);
            SDL_FreeSurface(surface);
        }
        created += cursors[kind] != 0;
    }
    return created;
}

void platform_cursor_kind_show(enum cursor_kind kind)
{
    SDL_Cursor *cursor = (uint32_t) kind < CURSOR_KINDS ? cursors[kind] : 0;
    SDL_SetCursor(cursor ? cursor : SDL_GetDefaultCursor());
}
