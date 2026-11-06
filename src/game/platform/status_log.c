// The game's status log, written to the port's log: a line is the text, then the argument string when given and the
// number (log_info; log_info_text has none), each followed by two spaces; texts are cut at 100 characters.

#include "platform/status_log.h"

#include <stdint.h>
#include <stdio.h>
#include "platform/log.h"

enum status_log_limits {
    STATUS_LOG_TEXT_MAX = 100,
};

static void log_line(const char *text, const char *arg, const char *number)
{
    platform_log("game log: %.*s%.*s%s%s", STATUS_LOG_TEXT_MAX, text ? text : "", STATUS_LOG_TEXT_MAX, arg ? arg : "", arg ? "  " : "", number);
}

void log_info(const char *text, const char *arg, int32_t number)
{
    char text_number[16];
    sprintf(text_number, "%d  ", number);
    log_line(text, arg, text_number);
}

void log_info_text(const char *text, const char *arg)
{
    log_line(text, arg, "");
}
