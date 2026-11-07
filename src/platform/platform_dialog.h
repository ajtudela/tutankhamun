// The platform's message box, used for the loaders' questions; its types and answers have MessageBox's values. The
// play mode shows an SDL message box.

#ifndef PLATFORM_DIALOG_H
#define PLATFORM_DIALOG_H

#include <stdint.h>

enum platform_box_type {
    PLATFORM_BOX_OK = 0,
    PLATFORM_BOX_OK_CANCEL = 1,
    PLATFORM_BOX_ABORT_RETRY_IGNORE = 2,
    PLATFORM_BOX_YES_NO_CANCEL = 3,
    PLATFORM_BOX_YES_NO = 4,
    PLATFORM_BOX_RETRY_CANCEL = 5,
    PLATFORM_BOX_TYPE_MASK = 0x0000000F,
    PLATFORM_BOX_ICON_ERROR = 0x00000010,
    PLATFORM_BOX_ICON_QUESTION = 0x00000020,
    PLATFORM_BOX_ICON_WARNING = 0x00000030,
    PLATFORM_BOX_ICON_INFORMATION = 0x00000040,
    PLATFORM_BOX_ICON_MASK = 0x000000F0,
    PLATFORM_BOX_TOPMOST = 0x00040000,
};

enum platform_box_answer {
    PLATFORM_ANSWER_OK = 1,
    PLATFORM_ANSWER_CANCEL = 2,
    PLATFORM_ANSWER_ABORT = 3,
    PLATFORM_ANSWER_RETRY = 4,
    PLATFORM_ANSWER_IGNORE = 5,
    PLATFORM_ANSWER_YES = 6,
    PLATFORM_ANSWER_NO = 7,
};

int32_t platform_message_box(void *window, const char *text, const char *caption, uint32_t type);

#endif
