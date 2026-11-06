// The window procedure of the game's main window (window_proc.c).

#ifndef WINDOW_WINDOW_PROC_H
#define WINDOW_WINDOW_PROC_H

#include <stdint.h>

intptr_t __stdcall window_proc(void *hwnd, uint32_t message, uintptr_t wparam, intptr_t lparam);

#endif
