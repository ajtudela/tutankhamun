// The effect and figure sound channels: their wav files bound into the Miles name list at start-up, -1 for a file
// missing from the game's directory.

#include "sound/effect.h"

#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include <stdio.h>

static const struct { int32_t id; const char *path; } effect_binds[] = {
    { 0, "audio\\wavs\\button.wav" },
    { 1, "audio\\wavs\\no.wav" },
    { 2, "audio\\wavs\\dig.wav" },
    { 3, "audio\\wavs\\fire.wav" },
    { 4, "audio\\wavs\\crash.wav" },
    { 5, "audio\\wavs\\splash.wav" },
    { 6, "audio\\wavs\\menu.wav" },
    { 7, "audio\\wavs\\build.wav" },
    { 8, "audio\\wavs\\build.wav" },
    { 9, "audio\\wavs\\fanfare1.wav" },
    { 10, "audio\\wavs\\fanfare2.wav" },
    { 17, "audio\\wavs\\crash.wav" },
    { 18, "audio\\wavs\\fire.wav" },
    { 11, "audio\\wavs\\ship_collision_beam.wav" },
    { 12, "audio\\wavs\\ship_collision_fore_quarter.wav" },
    { 13, "audio\\wavs\\ship_collision_aft_quarter.wav" },
    { 14, "audio\\wavs\\ship_collision_bow.wav" },
    { 15, "audio\\wavs\\ship_collision_stern.wav" },
    { 16, "audio\\wavs\\ship_order_top_speed.wav" },
};

static const struct { int32_t type; int32_t variant; const char *path; } figure_binds[] = {
    { 0, 0, "audio\\wavs\\hyena_r.wav" },
    { 0, 1, "audio\\wavs\\hyenaroam.wav" },
    { 0, 2, "audio\\wavs\\hyenaatk.wav" },
    { 0, 3, "audio\\wavs\\hyenadie.wav" },
    { 1, 0, "audio\\wavs\\lion_r.wav" },
    { 1, 1, "audio\\wavs\\lion_r.wav" },
    { 1, 2, "audio\\wavs\\lion_r.wav" },
    { 1, 3, "audio\\wavs\\liondie.wav" },
    { 2, 0, "audio\\wavs\\scorpion_r.wav" },
    { 2, 1, "audio\\wavs\\scorpion_r.wav" },
    { 2, 2, "audio\\wavs\\scorpion_r.wav" },
    { 2, 3, "audio\\wavs\\scorpiondie.wav" },
    { 3, 0, "audio\\wavs\\asp_r.wav" },
    { 3, 1, "audio\\wavs\\asp_r.wav" },
    { 3, 2, "audio\\wavs\\asp_r.wav" },
    { 3, 3, "audio\\wavs\\aspdie.wav" },
    { 4, 0, "audio\\wavs\\ostrich_r.wav" },
    { 4, 1, "audio\\wavs\\ostrichroam.wav" },
    { 4, 3, "audio\\wavs\\ostrichdie.wav" },
    { 4, 4, "audio\\wavs\\ostrichrun.wav" },
    { 5, 0, "audio\\wavs\\antelop_r.wav" },
    { 5, 1, "audio\\wavs\\anteloproam.wav" },
    { 5, 3, "audio\\wavs\\antelopdie.wav" },
    { 5, 4, "audio\\wavs\\anteloprun.wav" },
    { 6, 0, "audio\\wavs\\hippo_r.wav" },
    { 6, 2, "audio\\wavs\\hippoatk.wav" },
    { 6, 3, "audio\\wavs\\hippodie.wav" },
    { 7, 0, "audio\\wavs\\croc_r.wav" },
    { 7, 2, "audio\\wavs\\crocatk.wav" },
    { 7, 3, "audio\\wavs\\crocdie.wav" },
    { 8, 0, "audio\\wavs\\bird_r.wav" },
    { 8, 1, "audio\\wavs\\birdroam.wav" },
    { 8, 3, "audio\\wavs\\birddie.wav" },
    { 9, 2, "audio\\wavs\\sentry_atk.wav" },
    { 10, 2, "audio\\wavs\\police_atk.wav" },
    { 11, 2, "audio\\wavs\\arch_atk.wav" },
    { 12, 2, "audio\\wavs\\spear_atk.wav" },
    { 13, 2, "audio\\wavs\\chariot_atk.wav" },
    { 13, 3, "audio\\wavs\\chariot_die.wav" },
    { 14, 2, "audio\\wavs\\hunt_ant_atk.wav" },
    { 15, 2, "audio\\wavs\\hunt_ost_atk.wav" },
    { 16, 2, "audio\\wavs\\hunt_bird_atk.wav" },
    { 18, 2, "audio\\wavs\\arch_atk.wav" },
    { 19, 2, "audio\\wavs\\sword_atk.wav" },
    { 20, 2, "audio\\wavs\\spear_atk.wav" },
    { 17, 2, "audio\\wavs\\ship_arrow.wav" },
    { 17, 3, "audio\\wavs\\ship_die.wav" },
    { 21, 3, "audio\\wavs\\die1.wav" },
    { 22, 3, "audio\\wavs\\die2.wav" },
    { 23, 3, "audio\\wavs\\die3.wav" },
    { 24, 3, "audio\\wavs\\die4.wav" },
    { 25, 3, "audio\\wavs\\die5.wav" },
    { 26, 3, "audio\\wavs\\die6.wav" },
    { 27, 3, "audio\\wavs\\die7.wav" },
    { 28, 3, "audio\\wavs\\die8.wav" },
    { 29, 3, "audio\\wavs\\dieg1.wav" },
    { 30, 3, "audio\\wavs\\dieg2.wav" },
    { 31, 3, "audio\\wavs\\dieg3.wav" },
    { 32, 3, "audio\\wavs\\dieg4.wav" },
};

int32_t sound_channel_load_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }
    int32_t channel = sound_lib_name_list_add(&sound_manager.name_list, path, 0);
    fclose(fp);
    return channel;
}

void sound_effect_bind_channel(int32_t id, const char *path)
{
    effect_channels[id] = sound_channel_load_file(path);
}

void sound_figure_bind_channel(int32_t type, const char *path, int32_t variant)
{
    figure_channels[(type * FIGURE_SOUND_VARIANTS) + variant] = sound_channel_load_file(path);
}

void sound_effects_init(void)
{
    for (int32_t i = 0; i < EFFECT_CHANNEL_COUNT; i++) {
        effect_channels[i] = -1;
    }
    for (int32_t i = 0; i < (int32_t) (sizeof(effect_binds) / sizeof(effect_binds[0])); i++) {
        sound_effect_bind_channel(effect_binds[i].id, effect_binds[i].path);
    }
}

void sound_figure_init_channels(void)
{
    for (int32_t i = 0; i < FIGURE_CHANNEL_COUNT; i++) {
        figure_channels[i] = -1;
    }
    for (int32_t i = 0; i < (int32_t) (sizeof(figure_binds) / sizeof(figure_binds[0])); i++) {
        sound_figure_bind_channel(figure_binds[i].type, figure_binds[i].path, figure_binds[i].variant);
    }
}
