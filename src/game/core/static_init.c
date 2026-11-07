// The start-up initialization of the global objects (game_static_init): file lists, monuments, the campaign, empire
// routes, event texts, scenario events, building repair and ferry queues, floods, the sound manager (shut down at
// exit), the music object and the image collection, in that order; and the reader of EVENTMSG.TXT into its key text
// table.
//
// A text continued on the next line ("\" then a line end) is looked for from the position "key index + 1" of the file
// rather than after the backslash.

#include "core/static_init.h"

#include "building/abu_simbel.h"
#include "building/caesareum.h"
#include "building/library.h"
#include "building/lighthouse.h"
#include "building/monument_static.h"
#include "core/queue.h"
#include "figure/ships_player.h"
#include "flood/flood_internal.h"
#include "graphics/graphics_data.h"
#include "graphics/image_collection.h"
#include "graphics/screen_fade.h"
#include "io/game_file.h"
#include "io/game_file_internal.h"
#include "map/farm.h"
#include "map/map_data.h"
#include "platform/log.h"
#include "scenario/campaign.h"
#include "scenario/campaign_load.h"
#include "scenario/scenario_internal.h"
#include "sound/music.h"
#include "sound/sound_data.h"
#include "sound/sound_lib.h"
#include "sound/sound_lib_samples.h"
#include "text/event_text.h"
#include "text/text_data.h"

#include <io.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const float SOUND_REVERB_ROOM = 0.5f;
static const float SOUND_REVERB_TIME = 0.06f;
static const float SOUND_REVERB_MIX = 0.5f;

enum static_init_values {
    VEGETATION_START_GROWTH = 0x19,
    VEGETATION_START_TREE_GROWTH = 0x23,
    REPAIR_QUEUE_COUNT = 10,
    REPAIR_QUEUES_WITH_ROOM = 5,
    REPAIR_QUEUE_ROOM = 20,
};

int32_t farm_static_init_max_progress(void)
{
    farm_max_progress = FARM_PROGRESS_PER_PART * FARM_PROGRESS_PARTS;
    return farm_max_progress;
}

void fade_static_init(void)
{
    fade_object.format = -1;
    fade_object.steps = 0;
    fade_object.pixels[1] = NULL;
    fade_object.pixels[0] = NULL;
}

int32_t vegetation_growth_state_static_init(void)
{
    vegetation_growth_state.marsh_growth_b = VEGETATION_START_GROWTH;
    vegetation_growth_state.tree_growth_a = VEGETATION_START_TREE_GROWTH;
    vegetation_growth_state.marsh_growth_a = 0;
    vegetation_growth_state.tree_growth_b = 0;
    vegetation_growth_state.trees_reachable = 0;
    vegetation_growth_state.reeds_reachable = 0;
    vegetation_growth_state.trees_present = 0;
    vegetation_growth_state.marsh_present = 0;
    return 0;
}

void event_phrases_static_init(void)
{
    event_phrases.tokens = message_tokens.entries;
}

void file_list_construct(file_list *self)
{
    self->offsets = 0;
    self->names = 0;
    self->count = 0;
    self->offsets_capacity = 0;
    self->names_capacity = 0;
    self->names_used = 0;
}

void file_list_destroy(file_list *self)
{
    if (self->names) {
        free(self->names);
    }
    if (self->offsets) {
        free(self->offsets);
    }
}

void empire_route_construct(empire_route *route)
{
    route->path_length = 0;
    empire_route_path_set(route, NULL);
    route->num_points = 0;
    route->path_capacity = 0;
    route->in_use = 0;
    route->step = 5;
    route->first_object = -1;
    route->last_object = -1;
    route->kind = 1;
}

static int32_t is_newline(char c)
{
    return c == '\r' || c == '\n';
}

static int32_t read_key_texts(key_text_table *self, const text_key *keys, char *buf, int32_t len, int32_t pass)
{
    int32_t index = 0;
    int32_t key = -1;
    int32_t size = 0;
    int32_t i = 0;
    while (i <= len) {
        uint8_t c = (uint8_t) buf[i];
        if (c == ';') {
            if (i > len) {
                return size;
            }
            while (!is_newline(buf[i])) {
                i++;
                if (i > len) {
                    return size;
                }
            }
            if (buf[i + 1] != buf[i] && i < len) {
                i++;
            }
        } else if (c <= 0x20) {
            i++;
        } else if (key >= 0) {
            if (c != '"') {
                return -1;
            }
            i++;
            if (pass) {
                self->values[key] = self->data + size;
            }
            char t = buf[i];
            while (t != '"') {
                if (t == '\\') {
                    int32_t j = index + 1;
                    int32_t line_end = 0;
                    for (; j <= len; j++) {
                        if (is_newline(buf[j])) {
                            line_end = 1;
                            break;
                        }
                        if ((uint8_t) buf[j] > 0x20) {
                            break;
                        }
                    }
                    if (line_end) {
                        if (buf[j + 1] != buf[j] && j < len) {
                            j++;
                        }
                        i += j;
                    } else {
                        if (pass) {
                            self->data[size] = '\\';
                        }
                        size++;
                        i++;
                    }
                } else {
                    if (pass) {
                        self->data[size] = t;
                    }
                    size++;
                    i++;
                }
                t = buf[i];
            }
            if (pass) {
                self->data[size] = 0;
            }
            size++;
            i++;
            key = -1;
        } else {
            if (!pass) {
                for (int32_t j = i; j <= len; j++) {
                    if ((uint8_t) buf[j] <= 0x20) {
                        buf[j] = 0;
                        break;
                    }
                }
            }
            index = 0;
            while (index < self->count && _stricmp(keys[index].name, buf + i) != 0) {
                index++;
            }
            if (index >= self->count) {
                return -1;
            }
            i += (int32_t) strlen(keys[index].name);
            key = index;
        }
    }
    return size;
}

void key_text_table_load(key_text_table *self, const char *filename, const text_key *keys)
{
    self->count = 0;
    self->values = 0;
    self->keys = keys;
    self->data = 0;
    self->spare_3 = 0;
    if (!keys) {
        return;
    }
    for (const text_key *k = keys; k->name; k++) {
        self->count++;
    }
    if (!self->count) {
        return;
    }
    self->values = malloc((size_t) self->count * sizeof(*self->values));
    if (!self->values) {
        self->count = 0;
        return;
    }
    int32_t index = 0;
    for (; index < self->count; index++) {
        self->values[index] = 0;
    }
    if (!filename) {
        return;
    }
    void *file = fopen(filename, file_mode_read_binary);
    if (!file) {
        return;
    }
    int32_t len = (int32_t) _filelength(_fileno(file));
    char *buf = malloc((size_t) len + 1);
    if (!buf) {
        fclose(file);
        return;
    }
    fread(buf, (uint32_t) len, 1, file);
    buf[len] = 0;
    fclose(file);

    int32_t size = 0;
    for (int32_t pass = 0; pass < 2; pass++) {
        if (pass) {
            self->data = malloc((size_t) size);
            if (!self->data) {
                break;
            }
        }
        size = read_key_texts(self, keys, buf, len, pass);
        if (size < 0) {
            break;
        }
    }
    free(buf);
}

void scenario_event_clear_amounts(void *e)
{
    scenario_event *event = e;
    event->data.invasion.first_formation_id = 0;
    event->data.invasion.home_x = 0;
    event->data.invasion.home_y = 0;
    event->data.invasion.layout = 0;
    event->data.invasion.destination_x = 0;
    event->data.invasion.destination_y = 0;
    event->data.invasion.destination_building_id = 0;
    event->data.invasion.ignore_soldiers = 0;
}

void scenario_event_set_defaults(void *e)
{
    scenario_event *event = e;
    event->id = -1;
    event->type = 0;
    event->month = 0;
    event->on_completed = -1;
    event->on_refusal = -1;
    event->on_too_late = -1;
    event->on_defeat = -1;
    event->flags = 2;
    event->year.min = 1;
    event->year.max = 2;
    event->year.fixed = -1;
    event->amount.min = 5;
    event->amount.max = 10;
    event->amount.fixed = -1;
    event->invasion.min = 0;
    event->invasion.max = 49;
    event->invasion.fixed = -1;
    event->item.fixed = 1;
    event->item.min = -1;
    event->item.max = -1;
    event->route.min = 1;
    event->route.max = 8;
    event->route.fixed = -1;
    event->months_initial = 2;
    event->months_left = 0;
    event->state = 0;
    event->is_active = 0;
    event->can_comply_shown = 0;
    event->parent_id = -1;
    event->phrase_var = 0;
    event->attack_target = 0;
    scenario_event_clear_amounts(e);
}

static void scenario_value_unset(scenario_value *v)
{
    v->value = -1;
    v->fixed = -1;
    v->min = -1;
    v->max = -1;
}

void scenario_event_construct(scenario_event *event)
{
    scenario_value_unset(&event->item);
    scenario_value_unset(&event->amount);
    scenario_value_unset(&event->year);
    scenario_value_unset(&event->route);
    scenario_value_unset(&event->invasion);
    scenario_event_set_defaults(event);
}

void scenario_events_construct(scenario_events_t *list)
{
    for (int32_t i = 0; i < SCENARIO_MAX_EVENTS; i++) {
        scenario_event_construct(&list->events[i]);
    }
    list->count = 0;
    list->editor_selected_id = -1;
    list->editor_scroll = 0;
    for (int32_t i = 0; i < SCENARIO_MAX_EVENTS; i++) {
        list->events[i].id = (int16_t) i;
    }
    scenario_event_clear_amounts(&list->events[SCENARIO_CHEAT_INVASION_EVENT]);
    list->events[SCENARIO_CHEAT_INVASION_EVENT].type = EVENT_TYPE_INVASION;
    list->earthquake_active = 0;
    list->reserved_1 = 0;
}

void campaign_construct(campaign *c)
{
    for (int32_t i = 0; i < CAMPAIGN_MAX_LINKS; i++) {
        campaign_link_init(&c->links[i]);
    }
    c->link_count = 0;
    c->mission_name_count = 0;
    memset(c->saved_game_path, 0, sizeof(c->saved_game_path));
}

void dyn_queue_construct(dyn_queue *q)
{
    q->head = 1;
    q->tail = 0;
    q->count = 0;
    q->items = 0;
    q->capacity = 0;
    q->flag = 0;
}

void dyn_queue_construct_capacity(dyn_queue *q, int32_t capacity)
{
    q->tail = 0;
    q->count = 0;
    q->items = 0;
    q->capacity = 0;
    q->flag = 0;
    q->head = 1;
    dyn_queue_reserve(q, capacity);
}

static void building_repair_queues_init(void)
{
    for (int32_t i = 0; i < REPAIR_QUEUES_WITH_ROOM; i++) {
        dyn_queue_construct_capacity(&dock_queue_get(i)->queue, REPAIR_QUEUE_ROOM);
    }
    dock_queue *rest = dock_queue_get(REPAIR_QUEUES_WITH_ROOM);
    int32_t rest_count = REPAIR_QUEUE_COUNT - REPAIR_QUEUES_WITH_ROOM;
    memset(rest, 0, rest_count * sizeof(dock_queue));
    for (int32_t i = 0; i < rest_count; i++) {
        dyn_queue_construct(&rest[i].queue);
    }
}

void ring_queue_construct(ring_queue *q)
{
    q->tail = 0;
    q->head = 0;
    q->in_use = 0;
}

void sound_lib_object_construct(sound_name_list *list)
{
    list->entries = NULL;
    list->count = 0;
    list->capacity = 0;
}

void sound_lib_manager_init(sound_lib_state *mgr)
{
    sound_lib_object_construct(&mgr->name_list);
    mgr->disabled = 0;
    mgr->handles = NULL;
    mgr->spare_handles = NULL;
    mgr->states = NULL;
    mgr->slots = NULL;
    mgr->handles_3d = NULL;
    mgr->states_3d = NULL;
    mgr->slots_3d = NULL;
    mgr->rate = 22050;
    mgr->bits = 16;
    mgr->channels = 2;
    memset(&mgr->request, -1, sizeof(mgr->request));
    memset(&mgr->spare_request, -1, sizeof(mgr->spare_request));
    mgr->stream0_volume = 100;
    mgr->spare_volume = 100;
    mgr->stream1_volume = 100;
    mgr->stream2_volume = 100;
    mgr->eax_available = 0;
    mgr->eax_environment = 0;
    for (int32_t i = 0; i < 3; i++) {
        mgr->streams[i] = 0;
    }
    for (int32_t i = 0; i < 26; i++) {
        mgr->environment_reverb[i][0] = sound_lib_defaults[3 * i];
        mgr->environment_reverb[i][1] = sound_lib_defaults[3 * i + 1];
        mgr->environment_reverb[i][2] = sound_lib_defaults[3 * i + 2];
    }
    mgr->reserved_2 = -1;
    mgr->reserved_1 = 1;
    mgr->reverb_on = 0;
    mgr->reverb[0] = SOUND_REVERB_ROOM;
    mgr->reverb[1] = SOUND_REVERB_TIME;
    mgr->reverb[2] = SOUND_REVERB_MIX;
    mgr->provider_3d_kind = 5;
    sound_lib_manager = mgr;
}

void sound_lib_manager_destroy(sound_lib_state *self)
{
    sound_lib_shutdown(self);
    sound_lib_name_list_clear(&self->name_list);
}

void sound_manager_shutdown(void)
{
    sound_lib_manager_destroy(&sound_manager);
}

void music_object_construct(music_state *music)
{
    music->range_first = 0;
    music->range_last = 0;
    music->previous_range_first = 0;
    music->next_track = 0;
    music->state = 0;
    music->wait_start_tick = 0;
    music->track_finished = 0;
    music->invasion_music_request = 0;
    for (int32_t i = 0; i < MUSIC_MISSIONS; i++) {
        music_mission *mission = &music->missions[i];
        mission->track_count = 0;
        memset(mission->tracks, 0, sizeof(mission->tracks));
        memset(mission->ambient, 0, sizeof(mission->ambient));
        memset(mission->ambient_after_track, 0, sizeof(mission->ambient_after_track));
    }
}

void game_static_init(void)
{
    file_list_construct(&file_list_saves);
    library_object_init(&library_object);
    caesareum_object_init(&caesareum_object);
    small_royal_tomb_object_init(&small_royal_tomb_object.base);
    medium_royal_tomb_object_init(&medium_royal_tomb_object.base);
    grand_royal_tomb_object_init(&grand_royal_tomb_object.base);
    large_royal_tomb_object_init(&large_royal_tomb_object.base);
    abu_simbel_object_init(&abu_simbel_object);
    lighthouse_object_init(&lighthouse_object);
    fade_static_init();
    campaign_construct(&campaign_object.campaign);
    file_list_construct(&map_file_list.files);
    for (int32_t i = 0; i < EMPIRE_ROUTES; i++) {
        empire_route_construct(&empire_map_routes.routes[i]);
    }
    event_phrases_static_init();
    key_text_table_load(&event_texts, "EVENTMSG.TXT", event_text_keys);
    scenario_events_construct(&scenario_events);
    building_repair_queues_init();
    vegetation_growth_state_static_init();
    farm_static_init_max_progress();
    floods_construct(&floods_data);
    floods_static_init_units();
    sound_lib_manager_init(&sound_manager);
    atexit(sound_manager_shutdown);
    music_object_construct(&music_object);
    game_file_init_save_chunk_table_ext();
    game_file_init_map_chunk_table_ext();
    for (int32_t i = 0; i < FERRY_QUEUES; i++) {
        ring_queue_construct(&ferry_queues[i].queue);
    }
    file_list_construct(&player_file_list);
    image_collection_init();
}
