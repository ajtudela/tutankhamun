// The run log: a text file opened once, one line per message.

#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

void platform_log_open(const char *path);
void platform_log(const char *format, ...);
// the open log's file, 0 when there is none (the bug report sends its last part)
const char *platform_log_path(void);

#endif
