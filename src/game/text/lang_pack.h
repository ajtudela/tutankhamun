// Language packs (lang_pack.c): translations of the game's texts laid over the English texts of the game's files, and
// the language chosen in the Options menu's language window.

#ifndef TEXT_LANG_PACK_H
#define TEXT_LANG_PACK_H

#include <stdint.h>

enum lang_pack_texts {
    // the Options menu's "Language" entry: a text of group 2 that the game's file does not have
    LANG_PACK_MENU_TEXT = 100,
};

// the languages that can be chosen: English (the game's own files) and the packs found in config/lang (<code>.json)
int32_t lang_pack_count(void);
// the language's own name in the game's characters
const char *lang_pack_name(int32_t index);
int32_t lang_pack_current(void);
// loads the language over the English texts and remembers it; 0 (the texts stay as they were) when it does not load
int32_t lang_pack_select(int32_t index);
// the language of the command line (--language: en, de, it, es, pl) instead of the remembered one for this run;
// 0 when the code is not a language
int32_t lang_pack_request(const char *code);
// the codes --language takes, for its message
const char *lang_pack_codes(void);
// the requested or remembered language, after the game's texts are loaded
void lang_pack_startup(void);
// the language's text of a group's entry, 0 when it has none (lang_get_string uses the game's file then)
const char *lang_pack_string(int32_t group, int32_t index);

#endif
