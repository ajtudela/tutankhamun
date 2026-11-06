// Sound start-up: the digital sound of Miles, the name list of the sample files (SOUND_NAME_LIST_SIZE names), the
// music configuration (music.txt) and the music for the population, then the wav tables of the city, ambient, effect
// and figure channels.

#include "sound/sound_init.h"

#include "sound/ambient.h"
#include "sound/city_channels.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"

enum sound_init_size {
    SOUND_NAME_LIST_SIZE = 500,
};

void sound_init(void)
{
    sound_digital_ok = sound_lib_init_digital(&sound_manager);
    sound_lib_name_list_init(&sound_manager.name_list, SOUND_NAME_LIST_SIZE);
    sound_music_load_config(&music_object);
    sound_music_select_by_population(&music_object, 1);
    ambient_current_type = -1;
    sound_city_init_building_channels();
    sound_city_init_ambient_channels();
    sound_effects_init();
    sound_figure_init_channels();
}
