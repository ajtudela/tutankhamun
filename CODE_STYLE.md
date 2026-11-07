# Code style

The sources are C99 (MSVC, 32 and 64-bit; GCC and Clang on Linux compile them as C11). `.clang-format` at the root does the formatting: 4 spaces, no tabs, the `{` of a
function on its own line and every other `{` on the line of its statement, braces around every body, a space after
a cast, the pointer star with the name, lines up to 160 columns.

## Files

- Each file starts with one short header comment: what the file holds in two to four lines and, only when needed,
  the quirks it keeps on purpose (code that looks wrong but matches the original game's behaviour), in plain words.
- No other comments: the names say what the code does. What a comment would say goes into a name: a value into an
  enum member, a block into a static function, a condition into a named variable or a small predicate.
- Includes: the file's own header first, then the others sorted.
- A .c file declares nothing: a function used by other files has its prototype in the module's header; a function
  used only in its file is `static` and defined before its first use.

## Expressions and statements

- Parenthesize the parts of an expression when it mixes operators or the order matters: `(a * 100) / b`,
  `(flags & MASK) != 0`, `(a && b) || c`. A single operator or a plain `+` / `-` chain needs none.
- One statement per line. A statement that does not fit in 160 columns is a sign to name its parts (a local for a
  repeated part, a helper), not to wrap it by hand.
- Get a record once, then work through the pointer: `city_player *player = city_player_current();` and then
  `player->...`, not the getter on every line. The local is named after what it is (`player`, `b`, `f`, `model`).
- No `goto`. A yes / no result or flag is `bool`, except in a record that is saved, whose layout is the save format.

## Names, constants and data

- A constant of a set (states, kinds, levels, message ids) is an `enum` member in the header of the module that owns
  the set, defined once. A constant of another type is a `static const` with its type.
- Macros only where C has nothing else: include guards, platform switches, `offsetof`, `#` / `##`. An alias, a view
  of memory or a computation is a variable, a struct field or a `static inline` function.
- Globals are used by their name and type: no casts of a global's address, no address arithmetic over a record; a
  record is a struct with named fields.
- Names say what a thing is or does, in the module's prefix (`building_`, `figure_`, `city_`, `map_`, ...).
- No stub functions and no parameters that are never used.
- Tables of game data (costs, timings, model values, layouts) live in `config/*.json`, loaded by `src/game/core/config*.c`,
  not in the code. States and codes that saved games store stay compile-time constants.

## Behaviour

The game logic matches the original game's, saved games included. A change keeps it so: a fix of a difference from
the original is welcome, a change of the game's rules is not (CONTRIBUTING.md).
