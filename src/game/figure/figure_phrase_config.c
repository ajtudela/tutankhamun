// The gameplay config table of the figures' speech (core/config.h): figure_speech_sounds.json, per figure type the
// speech sound of its phrases (or none, by the climate, by the temple's god, by the sound type).

#include "core/config.h"
#include "figure/figure_phrase.h"
#include "figure/figure_type.h"

static const config_field figure_speech_sound_fields[] = {
    { "speech", figure_speech_sounds, 1, 1, FIGURE_SPEECH_BY_SOUND_TYPE, 1000, 0 },
};

static const config_storage figure_speech_sound_storage[] = {
    { figure_speech_sounds, sizeof(figure_speech_sounds) },
};

const config_table figure_speech_sound_table = {
    "figure_speech_sounds.json",
    "type",
    config_figure_type_names,
    FIGURE_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(figure_speech_sound_fields, figure_speech_sound_storage),
};
