// A bug report or an idea sent from the game (report.c), as Akhenaten sends its own (src/window/report_bug.cpp): a
// JSON note posted to https://playshhhit.com/tutankhamun, where the report service keeps it and opens a GitHub issue
// in dalerank/tutankhamun. The note has the kind ("bug" or "idea"), the title and the text, the build, the system,
// the last part of the log and a PNG screenshot. It is sent by a thread; the form asks for its state.
// TUTANKHAMUN_REPORT_URL replaces the address for a test.

#ifndef PLATFORM_REPORT_H
#define PLATFORM_REPORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum report_state {
    REPORT_IDLE,
    REPORT_SENDING,
    REPORT_SENT,
    REPORT_FAILED,
} report_state;

int32_t report_supported(void);
// the texts are the game's single-byte characters; the screenshot (malloc'ed, may be 0) is taken over and freed
int32_t report_send(const char *kind, const char *title, const char *text, uint8_t *png, size_t png_size);
report_state report_current_state(void);
// after REPORT_FAILED: the answer's status, 0 when there was no answer
int32_t report_failed_status(void);
void report_forget(void);

#endif
