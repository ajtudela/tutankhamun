// The status log of status_log.c: a line with a number, or without one.

#ifndef PLATFORM_STATUS_LOG_H
#define PLATFORM_STATUS_LOG_H

#include <stdint.h>

void log_info(const char *text, const char *arg, int32_t number);
void log_info_text(const char *text, const char *arg);

#endif
