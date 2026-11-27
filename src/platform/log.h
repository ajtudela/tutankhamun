// The run log: a text file opened once, one line per message.

#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

#include <stdbool.h>

void platform_log_open(const char *path);
// carries what was logged to a new file (a game folder path on Android) and goes on there
bool platform_log_move(const char *path);
void platform_log(const char *format, ...);
void platform_log_flush(void);
void platform_log_flush_due(void);
// the open log's file, 0 when there is none (the bug report sends its last part)
const char *platform_log_path(void);

#endif
