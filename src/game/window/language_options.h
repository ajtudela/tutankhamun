// The language window of the Options menu (language_options.c): a row per language and a row back to the city.

#ifndef WINDOW_LANGUAGE_OPTIONS_H
#define WINDOW_LANGUAGE_OPTIONS_H

#include "graphics/button.h"
#include <stdint.h>

void window_language_options_show(void);
void window_language_options_draw(void);
generic_button *language_options_buttons(void);
int32_t language_options_button_count(void);

#endif
