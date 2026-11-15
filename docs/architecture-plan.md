# The architecture: what to fix

[← README](../README.md)

The plan of the structural work on the sources. None of its steps changes the game's behaviour or the saved games
(CONTRIBUTING.md): each one is a commit of its own, after which all four builds pass, and from phase 2 on the saved
game regression of phase 1 shows the same results before and after it.

## The sources today

Three layers:

- `src/app`: the program: `main`, the frame loop (`play.c`), SDL's events turned into the Win32-shaped messages the
  game reads, Miles on SDL_mixer, Bink on Akhenaten's decoder;
- `src/platform`: the system: files, window, messages, log, the program's own update, Discord;
- `src/game`: the game, about 1000 files and 155 000 lines; it does not include SDL, and talks to the program through
  `platform_message` and `platform/*.h`.

What holds well: the game knows nothing of SDL; the Win32 behaviour (the modal loops, peek / get, `WM_ACTIVATEAPP`)
is rebuilt in one place, `app/play.c`; the data tables are in `config/*.json`, loaded over a snapshot and checked by
`--check-configs`; saved games are chunk tables with converters by version (`io/chunk_io.h`), and 101
`static_assert(sizeof ...)` keep the saved records' layout.

## The problems

### 1. The global state is declared in a few large headers

22 `*_data.h` headers hold about 2170 `extern` declarations:

| header | extern | included directly by |
|---|---|---|
| `building/monument_data.h` | 593 | |
| `building/building_data.h` | 460 | 123 files |
| `window/window_data.h` | 355 | 123 files |
| `figure/figure_data.h` | 172 | |
| `widget/widget_data.h` | 115 | 109 files |
| `graphics/graphics_data.h` | 91 | |

The globals themselves are right: they are the original's layout and the saved games'. Their declarations are not:

- `building/building.h` (134 includes) and `map/grid.h` (147) include `building_data.h`, which brings about twenty
  more headers, `editor/editor_new_scenario.h` and `graphics/button.h` among them;
- `core/random.h` includes `figure/figure_data.h`: the core depends on the figures;
- a change of any table builds most of `src/game` again, and the includes do not show which module uses which state.

The files with the most includes: `window/window_draw.c` 82, `window/building_info.c` 80, `io/game_file.c` 76,
`window/window_input.c` 66, `game/tick.c` 62.

### 2. The platform layer depends on the game

- `src/platform/platform_screen.c` included the game's headers for its screen buffer, `updater.c` and
  `language_store.c` included `app/app_files.h` (fixed by 2.1 and 2.2);
- `tutankhamun_platform` still has `PRIVATE src/game` in CMakeLists.txt.

### 3. Two folders answer to `platform/`

`src/platform/` and `src/game/platform/` are both included as `"platform/xxx.h"`, since the include path has both
`src` and `src/game`. It works only because no file name is in both; an include does not say which layer it means,
and the first name in both would take the wrong file without an error (fixed by 2.3).

### 4. No tests

The aim, the original's logic byte for byte, has no check of its own beyond playing. The loading of `test.sav` also
logs `ERR:COMP1 Corrupt.`, `ERR:COMP Error uncompressing.` and four `crt: invalid parameter`, the same before and
after the changes of phase 1: the file itself is damaged (1.6).

### 5. Smaller ones

- `app/play.c` (1216 lines) holds the Win32 message emulation, keyboard and mouse, the Android touch input and
  on-screen keyboard, and the display;
- large tables in C: `building/monument_anim_data.c` (8868 lines), `building/pyramid_data.c` (2608),
  `building/royal_tomb_part_data.c` (1299); CODE_STYLE.md puts game data tables in `config/*.json`;
- strings kept as globals of a fixed size (`str_removed_manmades[24]`, `file_mode_read_binary[4]` in `io/io_data.c`),
  from the original's layout;
- 484 lines in 123 files are longer than the 160 columns of `.clang-format` (109 of 161-165, 235 of 166-180, 140
  longer, the longest 265), and the hand-wrapped expressions break before the operator where clang-format breaks
  after it: 218 of 1015 files differ from `clang-format` 19.1.5.

## The plan

### Phase 0: the CI

- [x] 0.1. `--check-configs` in the Windows workflow, Win32 and x64 (`db79082`).
- [x] 0.2. `tutankhamun_format.yml`: `git clang-format` 19.1.5 on the lines a push or a pull request changes,
  advisory: a difference is a warning, the run stays green (`d37dd84`).

### Phase 1: the safety net

- [x] 1.1. The game is the static library `tutankhamun_game`, linked into the program; the checks of calls against
  their prototypes (`TUTANKHAMUN_STRICT_CALLS`) apply to it and to the program (`cfd5ad3`).
- [x] 1.2. A headless runner, its own target `tutankhamun_sim` (`src/tools/sim_main.c`: the program's sources with
  it for `main.c`): `tutankhamun_sim <game dir> in.sav --days N --out out.sav [--seed S] [--log file]`. It starts the
  game with `app_startup`, loads the save, runs `game_tick_run` 51 times a game day (16 days a month), saves through
  the game's own save and sets `crt_rand_seed` from the seed. Sound and video go to Miles / Bink tables that do nothing;
  `platform_bind` gives the run its own clock (a fixed start, 40 ms a tick), since the predators reseed `rand()` from
  the clock as the original does: two runs of a save give the same file. The frame part (windows, the city window's
  random number per frame, music) and the mission autosave do not run. A year of a 141k city takes about 20 s.
- [x] 1.3. `tutankhamun_sim --diff-saves a.sav b.sav`: both files read chunk by chunk through `chunk_io` with this
  build's table; for each chunk that differs, its index, how many of its elements differ, and the first difference's
  offset, element and byte; exit 0 when they are the same, 1 when they differ.
- [x] 1.4. `tests/saves/`, `tests/reference/`, `tests/regression.json` and `tests/run_regression.py`: each of the 8
  saves (players' cities from Pharaoh Heaven: pyramids and an obelisk being built, invasions, canals and sea trade, a
  temple complex and the gods, a Cleopatra-era city, a ferry island, a royal tomb) is run by `tutankhamun_sim` for a
  game year (192 days, a flood each) and compared with its reference by `--diff-saves`; `--update` saves the references
  again. The references are the current build's and guard the refactoring; references saved by the original Pharaoh
  2.1 after the same days would check the logic itself, a work of its own. No game data in the repository: the run
  needs the user's copy (`--game` or `TUTANKHAMUN_GAME_DIR`), so it is local, or on a runner of our own. All 8 take
  about 25 s.
- [x] 1.5. The presets `linux-asan` and `linux-ubsan`, each with its flow running `tests/sanitizer_checks.sh` (the
  configs, the saves of `tests/saves/` read); the regression under both is clean locally. Fixed: a shift into the
  sign bit, a misaligned read of old empire records, `figure[-1]` of an empty ferry slot; the route searches' reads
  past a grid stay (`tests/ubsan.supp`).
- [x] 1.6. The `ERR:COMP1 Corrupt.` of `test.sav` found with `--diff-saves` and the chunk table: the file is damaged,
  the reader is right. `test.sav` (2023, version 147, a schema of 175 chunks) has 9775 bytes after chunk 146 that its
  schema does not list: 1463 bytes (915 of `FF`, 548 of zeros), chunks 147-174 with the city's data (the flood settings,
  the service visit grid, the rubble and moisture grids), 3682 zeros; then chunks 147-174 again, all zero, to the end of
  the file. The game reads chunk 147 at the first of those bytes, takes `FFFFFFFF` for its compressed size (the `crt:
  invalid parameter` of a read of 4 GB, then `ERR:COMP1` and `ERR:COMP`), and 148, 168 and 170 the same way. The 8 saves
  of `tests/saves/` read by their schemas end exactly at the end of the file.

### Phase 2: clean layers

- [x] 2.1. `platform_screen.c` without the game's headers: its callers are the program's (`app/play.c`,
  `app/video_bink.c`), so it became `app/app_screen.c`, which reads the game's buffer, size and pixel format and hands
  them to `platform_screen_update(pixels, width, height, is_565)` of `platform/screen.c`; its external buffer, never
  set, is gone.
- [x] 2.2. `updater.c` and `language_store.c` without `app/`: `main.c` reads `[update] check` and calls
  `updater_disable`; the language store, all ini and config folder, is `app/app_language.c` behind
  `platform/language_store.h`, as `app/media_audio.c` is behind `platform/platform_sound.h`.
- [x] 2.3. `src/game/platform` becomes `src/game/host` (`"host/host_window.h"`, ...), `platform_rect.h`, a type of
  `platform/platform_window.h`, moves into `src/platform`; the function prefixes stay for
  now.
- [x] 2.4. All of `src/platform` built as `tutankhamun_platform` (it held only `screen.c` and `sound_device.c`, the
  rest was built with the program), without `PRIVATE src/game`: an include of the game's headers in `src/platform` is
  a build error.

Done when `grep -rE '#include "(building|window|graphics|figure|map|city|app)/' src/platform` finds nothing.

### Phase 3: the declarations of the global state by module

The definitions stay in the `*_data.c` files, in their order and with their types; only the `extern` declarations
move, each into the header of the module that owns the data (`extern int32_t farm_harvest_months[40];` from
`building/building_data.h` into the farm's header).

- [x] 3.1. `core/` includes only `core/`, the platform, parson and the standard headers: `random_generator_state`
  and `string_number_buffer` are declared in `core/`, `static_init` and `config_names.c` moved into `game/`.
- [x] 3.2. `map/grid.h` without `building/building_data.h`: `pharaohmap_info` moves into `map/map_grid_types.h`,
  `map_info` and `map_grids` are declared in `map/grid.h`; 126 files include `building_data.h`, 130 `window_data.h`.
- [x] 3.3. `building/building.h` without `building_data.h`: `city_buildings` is declared there; three map files that
  use the building tables include `building_data.h` themselves; 128 files include it, 130 `window_data.h`.
- [ ] 3.4. One owner per commit: `figure_data.h`, `widget_data.h`, `graphics_data.h`, then `window_data.h`,
  `building_data.h` and `monument_data.h` in series by subsystem (pyramids, tombs, temple complexes, ...).
  - [x] `figure_data.h`: 42 of its 44 externs used elsewhere moved into the figure headers (the figure records and
    the shared figure state into `figure/figure.h`, the result tile of a map search into `map/grid.h`);
    `enemy_formation_layout_offsets` stays for its inline readers there, `route_candidates` for three modules.
- [ ] 3.5. An emptied `*_data.h` is removed, or kept for its `*_data.c` alone.

A one-off search lists for each `extern` of a `*_data.h` the modules that use it: one module takes it, several take
it in the header of the module it belongs to. Each commit: the MSVC Win32 build and the regression;
its message gives the number of files that still include `building_data.h` and `window_data.h`.

Phase 3 starts only with the regression of phase 1 in place.

### Phase 4: the rest

- [ ] 4.1. `app/play.c` in four: `play_messages.c` (the message bus and the modal loops), `play_input.c` (keyboard,
  mouse, `virtual_key`), `play_touch.c` (touch, long press, the Android on-screen keyboard), `play.c` (window,
  display, frame).
- [ ] 4.2. The large tables in C looked through: what the saved games keep or what is reached by offset stays, with the
  reason in the file's header; the rest goes into `config/*.json` like the other configs, checked by
  `--check-configs` (done for the enemy formation layouts: `config/enemy_formation_layouts.json`).
- [ ] 4.3. The fixed-size string globals of `io/io_data.c` that are neither saved nor reached by offset become
  literals.

### Phase 5: no `extern` in the headers

A header declares types and functions only. A global is `static` in the `.c` of the module that owns it, and the
other modules reach it through that module's functions.

- [ ] 5.1. A global used only where it is defined loses its `extern`.
- [ ] 5.2. A global used by one other file moves into it as `static`, `static const` when nothing writes it (done
  for 15 tables of `figure_data.c`: the animals', the explosions', the enemies' attack lists, ...).
- [ ] 5.3. A global used by several files is `static` in its module, with access functions: a pointer to the record
  or the table, a get and a set for a value (done for `city_figures` and `city_buildings`: `figure_get`,
  `figure_route_paths`, `figure_table_get`, `building_get`, `building_table_get`, no longer inline).
- [ ] 5.4. The saved game's chunk tables (`io/io_data.c`) take the addresses from the owners' functions (the figures',
  the route paths' and the buildings' chunks are set in `game_file_init_save_chunk_table_ext`).

Each step keeps the regression the same. The search of 2426 `extern`s in all the headers: 940 used only where
defined, 623 by one other file, 921 by one module.

| phase | commits | after |
|---|---|---|
| 0. CI | 2 | |
| 1. library, runner, regression, sanitizers | 5-6 | 0 |
| 2. layers | 4 | 1 |
| 3. declarations by module | 15-30 | 1, 2 |
| 4. the rest | 3-5 | 1 |
| 5. no `extern` in the headers | by module | 3 |
