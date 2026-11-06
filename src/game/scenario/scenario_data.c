// The built-in data of the campaign, the events and the reason phrases. scenario/scenario_data.h declares them.

#include "scenario/scenario_data.h"

char fmt_campaign_error[24] = "ERR: %s line %d: %s %s";

char fmt_choice_not_found[36] = "%s mission %d: choice %d not found";

char err_unrecognized[20] = "unrecognized line";

char err_max_missions[24] = "max missions exceeded";

char err_name_too_long[24] = "mission name too long";

char err_bad_text_id[12] = "bad text id";

char err_bad_y[12] = "bad y value";

char err_bad_x[12] = "bad x value";

char err_too_many_choices[20] = "too many choices";

char err_bad_background_id[20] = "bad background id";

char err_bad_merge_id[16] = "bad merge id";

char err_bad_path_id[12] = "bad path_id";

char err_bad_transition_id[24] = "bad transition text id";

char err_bad_intro_id[16] = "bad intro id";

char err_bad_mission_id[16] = "bad mission id";

char err_add_link[28] = "failed to add mission link";

char err_bad_mission_name[20] = "bad mission name";

const char campaign_key_choice[8] = "choice";

const char campaign_key_choicescreen[16] = "choicescreen";

const char campaign_key_mission[8] = "mission";

const char campaign_section_cleopatras_alexandria[24] = "[CLEOPATRAS_ALEXANDRIA]";

const char campaign_section_ancient_conquerors[24] = "[ANCIENT_CONQUERORS]";

const char campaign_section_ramses_ii[12] = "[RAMSES_II]";

const char campaign_section_valley_of_the_kings[24] = "[VALLEY_OF_THE_KINGS]";

const char campaign_section_new_kingdom[16] = "[NEW_KINGDOM]";

const char campaign_section_middle_kingdom[20] = "[MIDDLE_KINGDOM]";

const char campaign_section_old_kingdom[16] = "[OLD_KINGDOM]";

const char campaign_section_archaic_period[20] = "[ARCHAIC_PERIOD]";

const char campaign_section_predynastic_period[24] = "[PREDYNASTIC_PERIOD]";

const char campaign_section_mission_names[16] = "[MISSION_NAMES]";

const char file_mode_write_binary[4] = "wb";

int32_t scenario_current_event_index = -1;

char event_add_error_text[88] = "ERR:Could not add event\0"
                                "%s%s\0\0\0\0"
                                "%s%s%s\0\0"
                                "/\0\0\0"
                                " %d\0"
                                " %d-%d\0\0"
                                "-%d\0"
                                "+%d\0"
                                "%s \0"
                                "** \0"
                                "*%d \0\0\0\0"
                                "*** ";

const char str_reason_table_loaded[28] = " OK: reason table loaded";

const char str_reason_data_invalid[28] = "reason phrase data invalid";

char fmt_reason_too_many_lines[52] = "ERR: reason file has too many lines, expected %d";

const char str_no_reason_phrases_file[32] = "ERR:No auto reason phrases file";

const char reason_phrases_file_name[24] = "auto reason phrases.txt";

uint8_t reason_phrases[1944];

uint16_t dialog_phrase_flags;
