// Opening a document or a web page with the desktop's program: the scenario editor's guide and the main menu's web site
// link; and a web address of the main menu's link buttons, which only the play mode allows.

#ifndef PLATFORM_SHELL_H
#define PLATFORM_SHELL_H

#include <stdint.h>

void platform_shell_open(void *window, const char *operation, const char *file);
void platform_web_links_allow(int32_t allowed);
int32_t platform_web_links_allowed(void);
void platform_url_open(const char *url);

#endif
