// The tables of scenario/scenario_data.c.

#ifndef SCENARIO_SCENARIO_DATA_H
#define SCENARIO_SCENARIO_DATA_H

#include <stdint.h>

extern char fmt_campaign_error[24];

extern char fmt_choice_not_found[36];

extern char err_unrecognized[20];

extern char err_max_missions[24];

extern char err_name_too_long[24];

extern char err_bad_text_id[12];

extern char err_bad_y[12];

extern char err_bad_x[12];

extern char err_too_many_choices[20];

extern char err_bad_background_id[20];

extern char err_bad_merge_id[16];

extern char err_bad_path_id[12];

extern char err_bad_transition_id[24];

extern char err_bad_intro_id[16];

extern char err_bad_mission_id[16];

extern char err_add_link[28];

extern char err_bad_mission_name[20];

extern const char campaign_key_choice[8];

extern const char campaign_key_choicescreen[16];

extern const char campaign_key_mission[8];

extern const char campaign_section_cleopatras_alexandria[24];

extern const char campaign_section_ancient_conquerors[24];

extern const char campaign_section_ramses_ii[12];

extern const char campaign_section_valley_of_the_kings[24];

extern const char campaign_section_new_kingdom[16];

extern const char campaign_section_middle_kingdom[20];

extern const char campaign_section_old_kingdom[16];

extern const char campaign_section_archaic_period[20];

extern const char campaign_section_predynastic_period[24];

extern const char campaign_section_mission_names[16];

extern const char file_mode_write_binary[4];

extern int32_t scenario_current_event_index;

extern char event_add_error_text[88];

extern const char str_reason_table_loaded[28];

extern const char str_reason_data_invalid[28];

extern char fmt_reason_too_many_lines[52];

extern const char str_no_reason_phrases_file[32];

extern const char reason_phrases_file_name[24];

extern uint8_t reason_phrases[1944];

extern uint16_t dialog_phrase_flags;

#endif
