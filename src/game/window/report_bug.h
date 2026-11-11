// The bug report form (report_bug.c), as Akhenaten has it (ui_report_bug_window.js): opened by the locust button at
// the bottom of the city's sidebar (Akhenaten's report_bug button, pharaoh_general/interface_00086 = image group 378),
// a bug or an idea, its title and description, sent with the build, the log's end and a screenshot of the city taken
// when the form opens (platform/report.h). Shown only where the program can send it (Windows for now).

#ifndef WINDOW_REPORT_BUG_H
#define WINDOW_REPORT_BUG_H

#include <stdint.h>

void report_bug_sidebar_button_draw(void);
int32_t report_bug_sidebar_button_handle(void);

void report_bug_show(void);
void report_bug_close(void);
void report_bug_draw(void);
void report_bug_handle_input(void);

#endif
