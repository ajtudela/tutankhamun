// The status log of status_log.c: a line with a number, or without one.

#ifndef HOST_STATUS_LOG_H
#define HOST_STATUS_LOG_H

#include <stdint.h>

void log_info(const char *text, const char *arg, int32_t number);
void log_info_text(const char *text, const char *arg);

#endif
