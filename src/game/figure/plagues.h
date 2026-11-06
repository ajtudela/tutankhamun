// The plague citizen, locust swarm and mummy actions of figure/plagues.c.

#ifndef FIGURE_PLAGUES_H
#define FIGURE_PLAGUES_H

enum plague_citizen_limit {
    PLAGUE_CITIZEN_FRAMES = 12,
    PLAGUE_CITIZEN_ROAD_RADIUS = 2,
};

enum locust_limit {
    LOCUST_FRAMES = 6,
};

enum mummy_limit {
    MUMMY_FRAMES = 24,
    MUMMY_ROAD_RADIUS = 6,
    MUMMY_ROAM_MIN = 25,
    MUMMY_ROAM_SPREAD = 50,
    MUMMY_CORPSE_IMAGES = 8,
};

void figure_plague_citizen_action(void);
void figure_locust_action(void);
void figure_mummy_action(void);

#endif
