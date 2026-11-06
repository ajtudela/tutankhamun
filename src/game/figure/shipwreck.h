// The shipwreck and bubbles actions of figure/shipwreck.c.

#ifndef FIGURE_SHIPWRECK_H
#define FIGURE_SHIPWRECK_H

enum bubbles_limit {
    BUBBLES_LAST_TICK = 21,
};

enum shipwreck_limit {
    SHIPWRECK_FRAMES = 36,
    SHIPWRECK_WAIT_SETTLED = 1000,
};

void figure_bubbles_action(void);
void figure_shipwreck_action(void);

#endif
