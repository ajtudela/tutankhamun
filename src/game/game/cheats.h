// The cheat table: a name with every character XOR 3, the function the cheat runs and whether it may be used while
// cheats are disabled.

#ifndef GAME_CHEATS_H
#define GAME_CHEATS_H

#include <stdbool.h>
#include <stdint.h>

enum {
    CHEAT_COUNT = 34,
};

typedef void (*cheat_function)(void);

typedef struct {
    const char *name;
    cheat_function fn;
    uint8_t allowed;
    uint8_t pad[3];
} cheat_entry;

int32_t game_cheat_find(const char *text);
bool game_cheat_execute(int32_t index);
bool game_cheat_apply_input(void);
bool game_cheat_input_close(void);
int32_t game_cheat_handle_key(int32_t message, int32_t key);
void cheat_life_from_death(void);
void cheat_bounty(void);
void cheat_mummys_curse(void);
void cheat_underworld(void);
void cheat_pharaohs_glory(void);
void cheat_sun_disk(void);
void cheat_mesektet(void);
void cheat_bird_of_prey(void);
void cheat_supreme_craftsman(void);
void cheat_noble_djed(void);
void cheat_big_dave(void);
void cheat_grenow(void);
void cheat_spirit_of_typhon(void);
void cheat_typhonian_relief(void);
void cheat_fury_of_seth(void);
void cheat_seth_strikes(void);
void cheat_cat_nip(void);
void cheat_meow(void);
void cheat_cat_fight(void);
void cheat_kitty_litter(void);
void cheat_crop_busters(void);
void cheat_amphibious_assault(void);
void cheat_hail_to_the_chief(void);
void cheat_crimson_tide(void);
void cheat_ancient_astronauts(void);
void cheat_jail_break(void);
void cheat_mummys_revenge(void);
void cheat_side_show(void);
void cheat_hippo_stomp(void);
void cheat_pharaohs_tomb(void);
void cheat_treasure_chest(void);
void cheat_mockattack1(void);
void cheat_mockattack2(void);

#endif
