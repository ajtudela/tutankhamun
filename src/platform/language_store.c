// The chosen language in the program's ini and the config folder of the language packs (language_store.h).

#include "platform/language_store.h"

#include "app/app_files.h"

static const char language_ini_section[] = "language";
static const char language_ini_key[] = "code";

void language_store_read(char *code, size_t size)
{
    code[0] = 0;
    char ini[1024];
    if (!app_ini_path(ini, sizeof(ini)) || !ini[0]) {
        return;
    }
    app_ini_read_text(ini, language_ini_section, language_ini_key, code, size);
}

int32_t language_store_write(const char *code)
{
    char ini[1024];
    if (!app_ini_path(ini, sizeof(ini)) || !ini[0]) {
        return 0;
    }
    return app_ini_write(ini, language_ini_section, language_ini_key, code) ? 1 : 0;
}

int32_t language_store_config_dir(char *dir, size_t size)
{
    return app_config_dir(dir, size) ? 1 : 0;
}
