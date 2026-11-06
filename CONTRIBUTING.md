# Contributing

When contributing to this repository, please first discuss the change you wish to make via an issue, the Discord
chat or any other method with the owner of this repository before opening a pull request.

## Aim of the project

This project is not meant for further development: it exists only to preserve the heritage of the original game,
Pharaoh + Cleopatra, and to port it to other platforms. The game logic is the original's, with its saved games and
its gameplay bugs.

Pull requests are accepted only for minimal fixes:

- bugs where the game behaves differently from the original;
- crashes;
- build and platform fixes;
- ports to other platforms.

New features and gameplay changes are not accepted.

## Pull requests

- One fix per pull request, as small as the fix allows; no reformatting of code the fix does not touch.
- Say what was wrong and how you checked the fix (the mission or saved game where it shows, before and after).
- The code follows [CODE_STYLE.md](CODE_STYLE.md) and the repository's [.clang-format](.clang-format).
- Saved games must stay compatible with the original game: a change to a saved record is not a minimal fix.
- Files taken from other projects keep their licence and header (LICENSES/README.md).
