// The protestor action of figure/protestor.c.

#ifndef FIGURE_PROTESTOR_H
#define FIGURE_PROTESTOR_H

enum protestor_timing {
    PROTESTOR_ANIMATION_FRAMES = 64,
    PROTESTOR_LIFETIME_TICKS = 200,
};

void figure_protestor_action(void);

#endif
