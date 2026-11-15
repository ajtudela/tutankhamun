// The chosen language of the game's texts (language_store.c): remembered in the program's ini ([language] code=de),
// and the config folder its packs (config/lang/<code>.json) are read from.

#ifndef PLATFORM_LANGUAGE_STORE_H
#define PLATFORM_LANGUAGE_STORE_H

#include <stddef.h>
#include <stdint.h>

// the remembered code, "" when none
void language_store_read(char *code, size_t size);
int32_t language_store_write(const char *code);
// 1 when the config folder is known
int32_t language_store_config_dir(char *dir, size_t size);

#endif
