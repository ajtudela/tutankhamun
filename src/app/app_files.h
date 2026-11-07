// The program's own files: its path and folder, the folder of the gameplay configs and tutankhamun.ini with its
// values (app_files.c). On Windows the configs and the ini are beside tutankhamun.exe; elsewhere the ini is
// tutankhamun/tutankhamun.ini in the user's configuration folder ($XDG_CONFIG_HOME, else ~/.config), and the configs
// are its tutankhamun/config when the user made one, else beside the program or in its share/tutankhamun/config (an
// installed tree, the AppImage).

#ifndef APP_APP_FILES_H
#define APP_APP_FILES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool app_program_path(char *path, size_t size);
bool app_program_dir(char *dir, size_t size);
bool app_config_dir(char *dir, size_t size);
bool app_ini_path(char *path, size_t size);

int32_t app_ini_read_int(const char *ini, const char *section, const char *key, int32_t fallback);
void app_ini_read_text(const char *ini, const char *section, const char *key, char *value, size_t size);
bool app_ini_write(const char *ini, const char *section, const char *key, const char *value);

#endif
