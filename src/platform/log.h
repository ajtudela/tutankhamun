// The run log: a text file opened once, one line per message.

#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

void platform_log_open(const char *path);
void platform_log(const char *format, ...);

#endif
