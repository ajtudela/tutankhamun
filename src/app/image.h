#ifndef APP_IMAGE_H
#define APP_IMAGE_H

// The program's run time and start-up: the run time set up before the game's code runs, the game's static
// initializers, and the start-up with its display mode request and the missing-configs message box.

#include <stdint.h>

int32_t image_init(void);

void image_run_static_initializers(void);

int32_t app_startup(void);
void app_report_no_configs_in_box(void);
void app_request_display_mode(int32_t mode);

#endif
