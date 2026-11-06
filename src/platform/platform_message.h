// The platform's window messages. The play mode answers the message peeks of the game's own modal loops (the message
// box, the video loop) with its SDL event pump.

#ifndef PLATFORM_MESSAGE_H
#define PLATFORM_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>

enum platform_message_id {
    PLATFORM_MSG_DESTROY = 0x0002,
    PLATFORM_MSG_MOVE = 0x0003,
    PLATFORM_MSG_SIZE = 0x0005,
    PLATFORM_MSG_ACTIVATE = 0x0006,
    PLATFORM_MSG_PAINT = 0x000F,
    PLATFORM_MSG_CLOSE = 0x0010,
    PLATFORM_MSG_ACTIVATE_APP = 0x001C,
    PLATFORM_MSG_SET_CURSOR = 0x0020,
    PLATFORM_MSG_DISPLAY_CHANGE = 0x007E,
    PLATFORM_MSG_KEY_DOWN = 0x0100,
    PLATFORM_MSG_KEY_UP = 0x0101,
    PLATFORM_MSG_CHAR = 0x0102,
    PLATFORM_MSG_SYS_KEY_DOWN = 0x0104,
    PLATFORM_MSG_MOUSE_MOVE = 0x0200,
    PLATFORM_MSG_LEFT_DOWN = 0x0201,
    PLATFORM_MSG_LEFT_UP = 0x0202,
    PLATFORM_MSG_LEFT_DOUBLE_CLICK = 0x0203,
    PLATFORM_MSG_RIGHT_DOWN = 0x0204,
    PLATFORM_MSG_RIGHT_UP = 0x0205,
    PLATFORM_MSG_RIGHT_DOUBLE_CLICK = 0x0206,
};

typedef struct {
    uint32_t message;
    uint64_t native[6];
} platform_message;

bool platform_message_peek(platform_message *msg, uint32_t first, uint32_t last, bool remove);
bool platform_message_get(platform_message *msg);
void platform_message_wait(void);
void platform_message_dispatch(const platform_message *msg);
bool platform_message_post(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam);
void platform_message_post_quit(int32_t code);
intptr_t platform_message_default(void *window, uint32_t message, uintptr_t wparam, intptr_t lparam);

#endif
