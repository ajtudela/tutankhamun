// Shim for Akhenaten's core/log.h, as much of it as bink.cpp uses (logs::info, logs::error): the lines go to the
// log function bink_c.cpp was given (akh_bink_set_log), or nowhere. Not Akhenaten code.
#pragma once

namespace logs {
void info(const char *format, ...);
void error(const char *format, ...);
}
