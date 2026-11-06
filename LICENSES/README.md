# Licence texts

The project's own code is licensed under the MIT License (`LICENSE` at the top of the repository). The files taken
from other projects keep their licence, named in the header of each file with its origin (the table below lists
them).

Files under the GNU Affero General Public License v3.0 (text: AGPL-3.0.txt). A program built with them is distributed
under AGPL-3.0 as a whole: every build of tutankhamun contains all of them.

| part | from | licence | text |
|---|---|---|---|
| src/third_party/akhenaten_bink/bink.cpp, bink.h | Akhenaten (github.com/dalerank/Akhenaten) | AGPL-3.0 | AGPL-3.0.txt (the same text as Akhenaten's LICENSE.txt) |
| cmake/BuildSDL2.cmake, cmake/BuildSDL2_mixer.cmake, cmake/TutankhamunBuildSDL.cmake | Akhenaten | AGPL-3.0 | AGPL-3.0.txt |
| src/platform/sound_device.c, src/platform/screen.c | Julius (github.com/bvschaik/julius) | AGPL-3.0 | AGPL-3.0.txt (the same text as Julius' LICENSE.txt) |
| src/game/window/display_resolution_list.c (the list drawn as Julius' select list, the entries as Akhenaten's display options) | Julius, Akhenaten | AGPL-3.0 | AGPL-3.0.txt |
| src/game/window/main_menu_link_icons.c (the Discord and Patreon pictures; the Patreon picture's button opens the author's site, dalerank.github.io) | Akhenaten | AGPL-3.0 | AGPL-3.0.txt |
| src/platform/discord_rpc.c, discord_rpc.h (the Discord presence over the client's pipe, a C port of src/platform/discord_rpc.cpp, commit 8ec540693) and src/game/game/discord_presence.c (its content, as src/scripts/discord_presence.js has it) | Akhenaten | AGPL-3.0 | AGPL-3.0.txt |
| src/platform/cursor.c, cursor.h (the play build's cursors: the arrow, shovel and sword pixel art, which Akhenaten has from Julius, and their SDL colour cursors) | Akhenaten, Julius | AGPL-3.0 | AGPL-3.0.txt |

Other code in the tree:

| part | from | licence | text |
|---|---|---|---|
| src/third_party/parson/parson.c, parson.h (version 1.5.3) | parson (github.com/kgabis/parson) | MIT | src/third_party/parson/LICENSE |
| the GitHub picture in src/game/window/main_menu_link_icons.c | Simple Icons (simpleicons.org) | CC0-1.0 | |

Libraries fetched at configure time and linked statically into tutankhamun (not in the tree; texts copied from the
fetched sources):

| library | version | licence | text |
|---|---|---|---|
| SDL2 (github.com/libsdl-org/SDL) | 2.32.10 | zlib | SDL2.txt |
| SDL2_mixer (github.com/libsdl-org/SDL_mixer) | 2.8.0 | zlib | SDL2_mixer.txt |
| minimp3 (github.com/lieff/minimp3, vendored in SDL2_mixer src/codecs/minimp3; the only codec besides WAVE built in) | as in SDL2_mixer 2.8.0 | CC0-1.0 | minimp3.txt |
