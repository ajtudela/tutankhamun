// The campaign file (campaign.txt), read at start-up and by the editor: the mission names and the nine periods with
// their "mission", "choicescreen" and "choice" lines, parsed into the campaign object's links. An error is logged and
// skips its line. Also the search of the link a choice leads to (a link with path 0 matches any path).

#include "scenario/campaign_load.h"

#include "io/io_data.h"
#include "platform/log.h"
#include "platform/platform_file.h"
#include "platform/status_log.h"
#include "scenario/campaign.h"
#include "scenario/scenario_data.h"
#include "window/window_data.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum campaign_file_limits {
    CAMPAIGN_LINE_BYTES = 200,
    CAMPAIGN_ERROR_BYTES = 0x12C,
    CAMPAIGN_PATH_MAX = 0xFF,
    CHOICE_SCREEN_MISSION = 0x7F,
    CHOICE_DEFAULT_SPACING = 50,
};

static const char campaign_delimiters[] = ",=";

void campaign_strip_whitespace(char *text)
{
    while (*text) {
        if ((uint8_t) *text > ' ') {
            text++;
            continue;
        }
        for (char *p = text; *p; p++) {
            p[0] = p[1];
        }
    }
}

int32_t campaign_parse_int(int32_t *out)
{
    char *token = strtok(0, campaign_delimiters);
    if (!token) {
        return 0;
    }
    const char *p = token;
    if (*p == '-') {
        p++;
    }
    for (; *p; p++) {
        if (!isdigit((uint8_t) *p)) {
            return 2;
        }
    }
    *out = atoi(token);
    return 1;
}

int32_t campaign_parse_short(int16_t *out)
{
    int32_t value;
    int32_t result = campaign_parse_int(&value);
    if (result == 1) {
        *out = (int16_t) value;
    }
    return result;
}

void campaign_log_error(const char *path, int32_t line, const char *text, const char *error)
{
    char message[CAMPAIGN_ERROR_BYTES];
    sprintf(message, fmt_campaign_error, path, line, text, error);
    log_info_text(message, 0);
}

enum campaign_section {
    SECTION_MISSION_NAMES = 0,
    SECTION_MISSION = 10,
    SECTION_CHOICE_SCREEN = 11,
    SECTION_CHOICE = 12,
    SECTION_COUNT = 13,
};

static const char *const section_names[SECTION_COUNT] = {
    campaign_section_mission_names,
    campaign_section_predynastic_period,
    campaign_section_archaic_period,
    campaign_section_old_kingdom,
    campaign_section_middle_kingdom,
    campaign_section_new_kingdom,
    campaign_section_valley_of_the_kings,
    campaign_section_ramses_ii,
    campaign_section_ancient_conquerors,
    campaign_section_cleopatras_alexandria,
    campaign_key_mission,
    campaign_key_choicescreen,
    campaign_key_choice,
};

static const char *close_link(campaign *self, campaign_link *link)
{
    if (link->mission_id < 0) {
        return 0;
    }
    if (!campaign_add_link(self, link)) {
        return empty_text;
    }
    link->choice_count = 0;
    link->mission_id = -1;
    link->intro_id = -1;
    link->transition_text_id = -1;
    link->path_id = 0;
    link->merge_count = 0;
    return 0;
}

static const char *read_mission(campaign *self, campaign_link *link)
{
    int32_t value;
    if (campaign_parse_int(&link->mission_id) != 1) {
        return err_bad_mission_id;
    }
    if (link->mission_id < 0 || link->mission_id >= self->mission_name_count) {
        return err_bad_mission_id;
    }
    if (campaign_parse_int(&link->intro_id) != 1) {
        return err_bad_intro_id;
    }
    if (campaign_parse_int(&link->transition_text_id) != 1) {
        return err_bad_transition_id;
    }
    int32_t result = campaign_parse_int(&value);
    if (result == 2) {
        return err_bad_path_id;
    }
    if (result == 0) {
        return 0;
    }
    if (value < 0 || value > CAMPAIGN_PATH_MAX) {
        return err_bad_path_id;
    }
    link->path_id = (uint8_t) value;
    link->merge_count = 0;
    for (int32_t i = 0; i < CAMPAIGN_LINK_MERGES; i++) {
        if (!strtok(0, campaign_delimiters)) {
            break;
        }
        result = campaign_parse_int(&value);
        if (result == 2) {
            return err_bad_merge_id;
        }
        if (result == 0) {
            break;
        }
        if (value < 0 || value > CAMPAIGN_PATH_MAX) {
            return err_bad_merge_id;
        }
        link->merge_ids[link->merge_count] = (uint8_t) value;
        link->merge_count++;
    }
    return 0;
}

static const char *read_choice(campaign_link *link)
{
    int32_t value;
    if (link->choice_count >= CAMPAIGN_LINK_CHOICES) {
        return err_too_many_choices;
    }
    if (campaign_parse_int(&value) != 1) {
        return err_bad_path_id;
    }
    if (value < 0 || value > CAMPAIGN_PATH_MAX) {
        return err_bad_path_id;
    }
    campaign_choice *choice = &link->choices[link->choice_count];
    choice->path_id = (uint8_t) value;
    if (campaign_parse_int(&choice->x) != 1) {
        return err_bad_x;
    }
    if (campaign_parse_int(&choice->y) != 1) {
        return err_bad_y;
    }
    if (campaign_parse_short(&choice->text_id) != 1) {
        return err_bad_text_id;
    }
    link->choice_count++;
    return 0;
}

static const char *read_mission_name(campaign *self, char *text)
{
    int32_t last = (int32_t) strlen(text) - 1;
    while (last >= 0 && (uint8_t) text[last] <= ' ') {
        text[last--] = 0;
    }
    if (last + 2 > CAMPAIGN_NAME_LENGTH) {
        return err_name_too_long;
    }
    if (last <= 0) {
        return 0;
    }
    if (self->mission_name_count >= CAMPAIGN_MAX_MISSIONS) {
        return err_max_missions;
    }
    strcpy(self->mission_names[self->mission_name_count], text);
    self->mission_name_count++;
    return 0;
}

void campaign_link_init(campaign_link *link)
{
    link->period = CAMPAIGN_PERIOD_NONE;
    link->path_id = 0;
    link->intro_id = -1;
    link->mission_id = -1;
    link->transition_text_id = -1;
    link->reserved_1 = -1;
    link->text_id = -1;
    link->background_id = -1;
    link->merge_count = 0;
    for (int32_t i = 0; i < CAMPAIGN_LINK_CHOICES; i++) {
        link->choices[i].x = CHOICE_DEFAULT_SPACING * (i + 1);
        link->choices[i].y = CHOICE_DEFAULT_SPACING * (i + 1);
        link->choices[i].text_id = -1;
        link->choices[i].path_id = 0;
    }
    link->choice_count = 0;
    link->played = 0;
}

bool campaign_add_link(campaign *self, const campaign_link *link)
{
    if (self->link_count >= CAMPAIGN_MAX_LINKS || link->period >= CAMPAIGN_PERIODS) {
        return 0;
    }
    self->links[self->link_count] = *link;
    self->link_count++;
    return 1;
}

int32_t campaign_find_link(campaign *self, int32_t period, int32_t index, int32_t choice)
{
    int32_t path;
    if (index < 0) {
        index = -1;
        path = 0;
    } else {
        if (index >= self->link_count) {
            return -1;
        }
        const campaign_link *from = &self->links[index];
        path = (from->choice_count > 0) ? from->choices[choice].path_id : from->path_id;
    }
    for (int32_t i = index + 1; i < self->link_count; i++) {
        const campaign_link *link = &self->links[i];
        if (link->period != period) {
            continue;
        }
        if (link->path_id == 0 || link->path_id == path) {
            return i;
        }
        for (int32_t m = 0; m < link->merge_count; m++) {
            if (link->merge_ids[m] == path) {
                return i;
            }
        }
    }
    return -1;
}

bool campaign_load(campaign *self, const char *path)
{
    campaign_link link = { 0 };
    campaign_link_init(&link);
    void *file = platform_fopen(path, file_mode_read_binary);
    if (!file) {
        return 0;
    }
    int32_t line_number = 0;
    int32_t in_names = 0;
    for (campaign_link *l = campaign_object.campaign.links; l < campaign_object.campaign.links + CAMPAIGN_MAX_LINKS; l++) {
        campaign_link_init(l);
    }
    self->link_count = 0;
    self->mission_name_count = 0;
    memset(self->saved_game_path, 0, sizeof(self->saved_game_path));
    bool ok = true;
    char line[CAMPAIGN_LINE_BYTES];
    char text[CAMPAIGN_LINE_BYTES];
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        strcpy(text, line);
        campaign_strip_whitespace(line);
        if (line[0] == ';' || line[0] == 0) {
            continue;
        }
        char *key = strtok(line, campaign_delimiters);
        if (!key) {
            continue;
        }
        int32_t section = 0;
        while (section < SECTION_COUNT && platform_text_compare_nocase(key, section_names[section]) != 0) {
            section++;
        }
        const char *error = 0;
        if (section == SECTION_MISSION_NAMES) {
            in_names = 1;
        } else if (section < SECTION_MISSION) {
            in_names = 0;
            error = close_link(self, &link);
            if (!error) {
                link.period = (uint8_t) (section - 1);
            }
        } else if (section == SECTION_MISSION) {
            if (in_names) {
                error = err_bad_mission_name;
            } else {
                error = close_link(self, &link);
                if (error) {
                    error = err_add_link;
                } else {
                    error = read_mission(self, &link);
                }
            }
        } else if (section == SECTION_CHOICE_SCREEN) {
            if (link.mission_id < 0) {
                link.mission_id = CHOICE_SCREEN_MISSION;
            }
            if (in_names) {
                error = err_bad_mission_name;
            } else if (campaign_parse_int(&link.background_id) != 1) {
                error = err_bad_background_id;
            } else if (campaign_parse_int(&link.text_id) != 1) {
                error = err_bad_text_id;
            }
        } else if (section == SECTION_CHOICE) {
            error = in_names ? err_bad_mission_name : read_choice(&link);
        } else {
            error = in_names ? read_mission_name(self, text) : err_unrecognized;
        }
        if (error) {
            campaign_log_error(path, line_number, text, error);
            ok = false;
        }
    }
    if (link.mission_id >= 0 && !campaign_add_link(self, &link)) {
        campaign_log_error(path, line_number, text, err_add_link);
        ok = false;
    }
    for (int32_t i = 0; i < self->link_count; i++) {
        const campaign_link *from = &campaign_object.campaign.links[i];
        if (from->choice_count <= 1) {
            continue;
        }
        for (int32_t c = 0; c < from->choice_count; c++) {
            if (campaign_find_link(&campaign_object.campaign, from->period, i, c) < 0) {
                sprintf(line, fmt_choice_not_found, section_names[from->period + 1], from->mission_id, from->choices[c].path_id);
                campaign_log_error(path, line_number, empty_text, line);
                ok = false;
            }
        }
    }
    memset(self->mission_records, 0, sizeof(self->mission_records));
    fclose(file);
    return ok;
}
