// Named slots of label_animation (one animation counter per label id): the options windows' labels, the redraw
// countdown, the file dialog's scroll position, the menu redraw flag, the warning labels, the advisor's opening
// animation and the game options label.

#ifndef GRAPHICS_LABEL_ANIMATION_SLOTS_H
#define GRAPHICS_LABEL_ANIMATION_SLOTS_H

enum label_animation_slot {
    LABEL_SLOT_DISPLAY_OPTIONS = 2,
    LABEL_SLOT_SOUND_OPTIONS = 3,
    LABEL_SLOT_SPEED_OPTIONS = 4,
    LABEL_SLOT_REFRESH_COUNTDOWN = 6,
    LABEL_SLOT_FILE_DIALOG_SCROLL = 7,
    LABEL_SLOT_MENU_DRAW = 8,
    LABEL_SLOT_WARNING_HOVER = 9,
    LABEL_SLOT_ADVISOR_OPEN = 14,
    LABEL_SLOT_GAME_OPTIONS = 15,
    LABEL_ANIMATION_SLOTS = 16,
};

#endif
