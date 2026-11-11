// The initial data of the windows as C tables: their buttons, menus, layouts and texts (declared in window_data.h).

#include "building/construction_ui.h"
#include "editor/editor_allowed_buildings.h"
#include "editor/editor_map_options.h"
#include "editor/editor_menu.h"
#include "editor/editor_monuments.h"
#include "editor/editor_scenario_settings.h"
#include "editor/editor_win_criteria.h"
#include "editor/empire_editor_object_panel.h"
#include "editor/empire_editor_objects.h"
#include "editor/empire_editor_routes.h"
#include "figure/action.h"
#include "figure/figure_phrase.h"
#include "game/tutorial.h"
#include "scenario/gift_buttons.h"
#include "text/lang.h"
#include "widget/city_view_rotate.h"
#include "widget/sidebar_buttons.h"
#include "window/advisor/financial.h"
#include "window/advisor/labor.h"
#include "window/advisor/military.h"
#include "window/advisor/population.h"
#include "window/advisor/religion.h"
#include "window/advisor/trade.h"
#include "window/advisor_comments.h"
#include "window/advisors.h"
#include "window/advisors_fleet.h"
#include "window/advisors_kingdom.h"
#include "window/advisors_monuments.h"
#include "window/building_info.h"
#include "window/building_info_military.h"
#include "window/building_info_ships.h"
#include "window/building_info_storage.h"
#include "window/building_info_storage_orders.h"
#include "window/empire_editor_input.h"
#include "window/empire_window.h"
#include "window/file_dialog_buttons.h"
#include "window/high_scores.h"
#include "window/input_dialogs.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/map_selection.h"
#include "window/message_box.h"
#include "window/message_dialog.h"
#include "window/message_dialog_buttons.h"
#include "window/message_list.h"
#include "window/mission_flow.h"
#include "window/name_list.h"
#include "window/options_buttons.h"
#include "window/overlay_menu.h"
#include "window/player_selection.h"
#include "window/popup_dialog.h"
#include "window/resource_settings.h"

#include <assert.h>

char setup_mp3_path[24] = "audio\\music\\setup.mp3";

const char save_file_pattern[8] = "\\*.sav";

const char file_mode_read[4] = "r";

const char path_separator[4] = "\\";

const char save_file_extension[4] = "sav";


uint8_t cheats_disabled = 1;

int32_t legion_cycle_last = 1;

const char str_err_reinit[36] = "ERR:Re-initializing direct draw.  ";

const char str_ok_not_active[20] = "OK :Not active.  ";

char mission_briefing_speech_files[2400] = "200_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "201_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "202_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "203_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "204_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "205_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "206_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "207_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "208_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "209_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "210_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "211_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "212_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "213_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "214_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "215_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "216_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "217_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "218_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "219_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "220_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "221_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "222_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "223_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "224_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "225_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "226_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "227_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "228_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "229_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "230_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "231_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "232_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "233_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "234_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "235_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "236_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                           "237_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C38_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C39_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C40_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C41_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C42_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C43_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C44_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C45_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C46_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C47_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C48_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C49_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C50_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C51_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C52_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                           "C53_mission.mp3";

char mission_briefing_speech_files_classic[2400] = "200_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "201_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "202_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "203_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "204_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "205_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "206_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "207_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "208_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "209_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "210_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "211_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "212_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "213_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "214_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "215_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "216_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "217_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "218_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "219_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "220_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "221_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "222_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "223_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "224_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "225_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "226_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "227_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "228_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "229_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "230_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "231_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "232_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "233_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "234_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "235_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "236_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                   "237_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C38_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C39_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C40_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C41_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C42_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C43_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C44_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C45_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C46_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C47_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C48_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C49_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C50_mission_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C51_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C52_mission.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                   "C53_mission.mp3";

char mission_victory_speech_files[2400] = "200_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "201_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "202_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "203_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "204_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "205_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "206_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "207_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "208_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "209_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "210_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "211_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "212_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "213_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "214_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "215_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "216_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "217_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "218_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "219_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "220_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "221_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "222_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "223_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "224_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "225_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "226_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "227_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "228_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "229_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "230_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "231_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "232_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "233_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "234_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "235_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "236_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                          "237_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C38_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C39_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C40_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C41_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C42_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C43_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C44_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C45_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C46_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C47_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C48_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C49_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C50_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C51_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C52_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                          "C53_victory.mp3";

char mission_victory_speech_files_classic[2404] = "200_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "201_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "202_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "203_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "204_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "205_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "206_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "207_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "208_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "209_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "210_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "211_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "212_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "213_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "214_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "215_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "216_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "217_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "218_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "219_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "220_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "221_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "222_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "223_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "224_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "225_victory_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "226_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "227_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "228_victory_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "229_victory_classic.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "230_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "231_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "232_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "233_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "234_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "235_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "236_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\000"
                                                  "237_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C38_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C39_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C40_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C41_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C42_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C43_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C44_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C45_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C46_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C47_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C48_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C49_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C50_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C51_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C52_victory.mp3\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                                                  "C53_victory.mp3";

generic_button main_menu_buttons[5] = { { 192, 100, 448, 125, main_menu_button_click, button_none, 0, 0, 0, 1,
                                                    0, 0 },
    { 192, 140, 448, 165, main_menu_button_click, button_none, 0, 0, 0, 1, 1, 0 },
    { 192, 180, 448, 205, main_menu_button_click, button_none, 0, 0, 0, 1, 2, 0 },
    { 192, 220, 448, 245, main_menu_button_click, button_none, 0, 0, 0, 1, 3, 0 },
    { 192, 260, 448, 285, main_menu_button_click, button_none, 0, 0, 0, 1, 4, 0 } };

generic_button game_menu_buttons[5] = { { 160, 128, 480, 153, main_menu_button_continue, button_none, 0, 0, 0, 1, 0, 0 },
      { 160, 176, 480, 201, main_menu_button_campaign, button_none, 0, 0, 0, 1, 0, 0 },
      { 160, 224, 480, 249, main_menu_button_load, button_none, 0, 0, 0, 1, 0, 0 },
      { 160, 272, 480, 297, main_menu_button_custom, button_none, 0, 0, 0, 1, 0, 0 },
      { 160, 320, 480, 345, main_menu_button_back, button_none, 0, 0, 0, 1, 0, 0 } };

uint32_t main_menu_button_labels[5] = { 0, 2, 3, 5, 4 };

const char str_replay_save[20] = "autosave_replay.sav";

const char str_video_win_big[32] = "Binks\\high\\win_grand_big.bik";

const char str_video_win_small[32] = "Binks\\low\\win_grand_small.bik";



char postfix_none[4] = " ";

menu_button_tables build_menu_buttons = {
    .build_menu = {
        { 0, 0, 256, 20, build_menu_set_type, button_none, 0, 0, 0, 1, 1, 0 },
        { 0, 24, 256, 44, build_menu_set_type, button_none, 0, 0, 0, 1, 2, 0 },
        { 0, 48, 256, 68, build_menu_set_type, button_none, 0, 0, 0, 1, 3, 0 },
        { 0, 72, 256, 92, build_menu_set_type, button_none, 0, 0, 0, 1, 4, 0 },
        { 0, 96, 256, 116, build_menu_set_type, button_none, 0, 0, 0, 1, 5, 0 },
        { 0, 120, 256, 140, build_menu_set_type, button_none, 0, 0, 0, 1, 6, 0 },
        { 0, 144, 256, 164, build_menu_set_type, button_none, 0, 0, 0, 1, 7, 0 },
        { 0, 168, 256, 188, build_menu_set_type, button_none, 0, 0, 0, 1, 8, 0 },
        { 0, 192, 256, 212, build_menu_set_type, button_none, 0, 0, 0, 1, 9, 0 },
        { 0, 216, 256, 236, build_menu_set_type, button_none, 0, 0, 0, 1, 10, 0 },
        { 0, 240, 256, 260, build_menu_set_type, button_none, 0, 0, 0, 1, 11, 0 },
        { 0, 264, 256, 284, build_menu_set_type, button_none, 0, 0, 0, 1, 12, 0 },
        { 0, 288, 256, 308, build_menu_set_type, button_none, 0, 0, 0, 1, 13, 0 },
        { 0, 312, 256, 332, build_menu_set_type, button_none, 0, 0, 0, 1, 14, 0 },
        { 0, 336, 256, 356, build_menu_set_type, button_none, 0, 0, 0, 1, 15, 0 },
        { 0, 360, 256, 380, build_menu_set_type, button_none, 0, 0, 0, 1, 16, 0 },
        { 0, 384, 256, 404, build_menu_set_type, button_none, 0, 0, 0, 1, 17, 0 },
        { 0, 408, 256, 428, build_menu_set_type, button_none, 0, 0, 0, 1, 18, 0 },
        { 0, 432, 256, 452, build_menu_set_type, button_none, 0, 0, 0, 1, 19, 0 },
        { 0, 456, 256, 476, build_menu_set_type, button_none, 0, 0, 0, 1, 20, 0 },
        { 0, 480, 256, 500, build_menu_set_type, button_none, 0, 0, 0, 1, 21, 0 },
        { 0, 504, 256, 524, build_menu_set_type, button_none, 0, 0, 0, 1, 22, 0 },
        { 0, 528, 256, 548, build_menu_set_type, button_none, 0, 0, 0, 1, 23, 0 },
        { 0, 552, 256, 572, build_menu_set_type, button_none, 0, 0, 0, 1, 24, 0 },
        { 0, 576, 256, 596, build_menu_set_type, button_none, 0, 0, 0, 1, 25, 0 },
        { 0, 600, 256, 620, build_menu_set_type, button_none, 0, 0, 0, 1, 26, 0 },
        { 0, 624, 256, 644, build_menu_set_type, button_none, 0, 0, 0, 1, 27, 0 },
        { 0, 648, 256, 668, build_menu_set_type, button_none, 0, 0, 0, 1, 28, 0 },
        { 0, 672, 256, 692, build_menu_set_type, button_none, 0, 0, 0, 1, 29, 0 },
        { 0, 696, 256, 716, build_menu_set_type, button_none, 0, 0, 0, 1, 30, 0 }
    },
    .overlay_menu = {
        { 0, 0, 160, 24, overlay_menu_select, button_none, 0, 0, 0, 1, 0, 0 },
        { 0, 24, 160, 48, overlay_menu_select, button_none, 0, 0, 0, 1, 1, 0 },
        { 0, 48, 160, 72, overlay_menu_select, button_none, 0, 0, 0, 1, 2, 0 },
        { 0, 72, 160, 96, overlay_menu_select, button_none, 0, 0, 0, 1, 3, 0 },
        { 0, 96, 160, 120, overlay_menu_select, button_none, 0, 0, 0, 1, 4, 0 },
        { 0, 120, 160, 144, overlay_menu_select, button_none, 0, 0, 0, 1, 5, 0 },
        { 0, 144, 160, 168, overlay_menu_select, button_none, 0, 0, 0, 1, 6, 0 },
        { 0, 168, 160, 192, overlay_menu_select, button_none, 0, 0, 0, 1, 7, 0 },
        { 0, 192, 160, 216, overlay_menu_select, button_none, 0, 0, 0, 1, 8, 0 },
        { 0, 216, 160, 240, overlay_menu_select, button_none, 0, 0, 0, 1, 9, 0 }
    },
    .overlay_submenu = {
        { 0, 0, 160, 24, overlay_submenu_select, button_none, 0, 0, 0, 1, 0, 0 },
        { 0, 24, 160, 48, overlay_submenu_select, button_none, 0, 0, 0, 1, 1, 0 },
        { 0, 48, 160, 72, overlay_submenu_select, button_none, 0, 0, 0, 1, 2, 0 },
        { 0, 72, 160, 96, overlay_submenu_select, button_none, 0, 0, 0, 1, 3, 0 },
        { 0, 96, 160, 120, overlay_submenu_select, button_none, 0, 0, 0, 1, 4, 0 },
        { 0, 120, 160, 144, overlay_submenu_select, button_none, 0, 0, 0, 1, 5, 0 },
        { 0, 144, 160, 168, overlay_submenu_select, button_none, 0, 0, 0, 1, 6, 0 },
        { 0, 168, 160, 192, overlay_submenu_select, button_none, 0, 0, 0, 1, 7, 0 },
        { 0, 192, 160, 216, overlay_submenu_select, button_none, 0, 0, 0, 1, 8, 0 },
        { 0, 216, 160, 240, overlay_submenu_select, button_none, 0, 0, 0, 1, 9, 0 }
    },
    .display_options = {
        { 144, 136, 336, 156, display_options_button_mode, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 160, 336, 180, display_options_button_resolution, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 184, 336, 204, display_options_button_resolution, button_none, 0, 0, 0, 1, 2, 0 },
        { 144, 208, 336, 228, display_options_button_resolution, button_none, 0, 0, 0, 1, 3, 0 },
        { 144, 232, 336, 252, window_go_city, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .display_options_end = 0,
    .game_options = {
        { 144, 116, 336, 136, game_options_button_toggle, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 140, 336, 160, game_options_button_toggle, button_none, 0, 0, 0, 1, 2, 0 },
        { 144, 164, 336, 184, game_options_button_toggle, button_none, 0, 0, 0, 1, 3, 0 },
        { 144, 188, 336, 208, game_options_button_toggle, button_none, 0, 0, 0, 1, 4, 0 },
        { 144, 212, 336, 232, game_options_button_toggle, button_none, 0, 0, 0, 1, 5, 0 },
        { 144, 236, 336, 256, game_options_button_toggle, button_none, 0, 0, 0, 1, 6, 0 },
        { 144, 260, 336, 280, game_options_button_toggle, button_none, 0, 0, 0, 1, 7, 0 },
        { 144, 284, 336, 304, game_options_button_toggle, button_none, 0, 0, 0, 1, 8, 0 },
        { 144, 308, 336, 328, game_options_button_toggle, button_none, 0, 0, 0, 1, 9, 0 },
        { 144, 332, 336, 352, game_options_button_toggle, button_none, 0, 0, 0, 1, 10, 0 },
        { 144, 356, 336, 376, game_options_button_toggle, button_none, 0, 0, 0, 1, 11, 0 },
        { 144, 380, 336, 400, game_options_button_toggle, button_none, 0, 0, 0, 1, 12, 0 },
        { 144, 428, 336, 448, game_options_button_toggle, button_none, 0, 0, 0, 1, 13, 0 },
        { 144, 452, 336, 472, window_go_city, button_none, 0, 0, 0, 1, 14, 0 }
    },
    .speed_options = {
        { 144, 232, 336, 252, window_go_city, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 262, 336, 282, speed_options_button_cancel, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .sound_options = {
        { 64, 162, 288, 182, sound_options_button_toggle, button_none, 0, 0, 0, 1, 1, 0 },
        { 64, 192, 288, 212, sound_options_button_toggle, button_none, 0, 0, 0, 1, 2, 0 },
        { 64, 222, 288, 242, sound_options_button_toggle, button_none, 0, 0, 0, 1, 3, 0 },
        { 64, 252, 288, 272, sound_options_button_toggle, button_none, 0, 0, 0, 1, 4, 0 },
        { 144, 296, 336, 316, window_go_city, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 326, 336, 346, sound_options_button_cancel, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .file_dialog_rows = {
        { 160, 128, 448, 144, file_dialog_button_row, button_none, 0, 0, 0, 1, 0, 0 },
        { 160, 144, 448, 160, file_dialog_button_row, button_none, 0, 0, 0, 1, 1, 0 },
        { 160, 160, 448, 176, file_dialog_button_row, button_none, 0, 0, 0, 1, 2, 0 },
        { 160, 176, 448, 192, file_dialog_button_row, button_none, 0, 0, 0, 1, 3, 0 },
        { 160, 192, 448, 208, file_dialog_button_row, button_none, 0, 0, 0, 1, 4, 0 },
        { 160, 208, 448, 224, file_dialog_button_row, button_none, 0, 0, 0, 1, 5, 0 },
        { 160, 224, 448, 240, file_dialog_button_row, button_none, 0, 0, 0, 1, 6, 0 },
        { 160, 240, 448, 256, file_dialog_button_row, button_none, 0, 0, 0, 1, 7, 0 },
        { 160, 256, 448, 272, file_dialog_button_row, button_none, 0, 0, 0, 1, 8, 0 },
        { 160, 272, 448, 288, file_dialog_button_row, button_none, 0, 0, 0, 1, 9, 0 },
        { 160, 288, 448, 304, file_dialog_button_row, button_none, 0, 0, 0, 1, 10, 0 },
        { 160, 304, 448, 320, file_dialog_button_row, button_none, 0, 0, 0, 1, 11, 0 }
    },
    .empire_editor_modes = {
        { 0, 30, 100, 48, empire_editor_button, button_none, 0, 0, 0, 1, 1, 0 },
        { 0, 50, 100, 68, empire_editor_button, button_none, 0, 0, 0, 1, 2, 0 },
        { 0, 70, 100, 88, empire_editor_button, button_none, 0, 0, 0, 3, 3, 0 },
        { 0, 90, 100, 108, empire_editor_button, button_none, 0, 0, 0, 3, 4, 0 },
        { 104, 30, 204, 48, empire_editor_button, button_none, 0, 0, 0, 3, 14, 0 },
        { 104, 50, 204, 68, empire_editor_button, button_none, 0, 0, 0, 3, 15, 0 },
        { 104, 70, 204, 88, empire_editor_file_button, button_none, 0, 0, 0, 3, 0, 0 },
        { 104, 90, 204, 108, empire_editor_button, button_none, 0, 0, 0, 3, 8, 0 }
    },
    .empire_editor_general = {
        { 8, 80, 128, 100, empire_editor_open_trade_prices, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .empire_editor_general_end = 0,
    .editor_start_year = {
        { 30, 56, 130, 86, editor_start_year_toggle_era, button_none, 0, 0, 0, 1, 0, 0 },
        { 150, 56, 270, 86, editor_start_year_button, button_none, 0, 0, 0, 1, 0, 0 },
        { 10, 80, 240, 105, window_go_city, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .editor_start_year_end = 0,
    .empire_editor_ornament_kinds = {
        { 170, 8, 350, 28, empire_editor_choose_image_kind, button_none, 0, 0, 0, 1, 0, 0 },
        { 170, 33, 350, 53, empire_editor_choose_image_kind, button_none, 0, 0, 0, 1, 1, 0 },
        { 170, 58, 350, 78, empire_editor_choose_image_kind, button_none, 0, 0, 0, 1, 2, 0 }
    },
    .empire_editor_ornament_kinds_end = 0,
    .empire_editor_routes = {
        { 170, 10, 350, 35, route_number_button, button_none, 0, 0, 0, 3, 0, 0 },
        { 170, 35, 350, 60, route_kind_button, button_none, 0, 0, 0, 3, 0, 0 },
        { 170, 60, 350, 85, route_step_button, button_none, 0, 0, 0, 3, 0, 0 },
        { 170, 85, 350, 110, route_delete_button, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .editor_trade_amounts = {
        { 25, 90, 75, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 0, 0 },
        { 75, 90, 125, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 1, 0 },
        { 125, 90, 175, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 2, 0 },
        { 175, 90, 225, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 3, 0 },
        { 225, 90, 275, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 4, 0 },
        { 275, 90, 325, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 5, 0 },
        { 325, 90, 375, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 6, 0 },
        { 375, 90, 425, 112, editor_trade_amount_next, button_none, 0, 0, 0, 1, 7, 0 },
        { 25, 150, 75, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 8, 0 },
        { 75, 150, 125, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 9, 0 },
        { 125, 150, 175, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 10, 0 },
        { 175, 150, 225, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 11, 0 },
        { 225, 150, 275, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 12, 0 },
        { 275, 150, 325, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 13, 0 },
        { 325, 150, 375, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 14, 0 },
        { 375, 150, 425, 172, editor_trade_amount_next, button_none, 0, 0, 0, 1, 15, 0 }
    },
    .editor_trade_prices = {
        { 34, 80, 69, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 1 },
        { 34, 105, 69, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 1 },
        { 69, 80, 104, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 2 },
        { 69, 105, 104, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 2 },
        { 104, 80, 139, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 3 },
        { 104, 105, 139, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 3 },
        { 139, 80, 174, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 4 },
        { 139, 105, 174, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 4 },
        { 174, 80, 209, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 5 },
        { 174, 105, 209, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 5 },
        { 209, 80, 244, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 6 },
        { 209, 105, 244, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 6 },
        { 244, 80, 279, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 7 },
        { 244, 105, 279, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 7 },
        { 279, 80, 314, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 8 },
        { 279, 105, 314, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 8 },
        { 314, 80, 349, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 9 },
        { 314, 105, 349, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 9 },
        { 349, 80, 384, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 10 },
        { 349, 105, 384, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 10 },
        { 384, 80, 419, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 11 },
        { 384, 105, 419, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 11 },
        { 419, 80, 454, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 12 },
        { 419, 105, 454, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 12 },
        { 454, 80, 489, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 13 },
        { 454, 105, 489, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 13 },
        { 489, 80, 524, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 14 },
        { 489, 105, 524, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 14 },
        { 524, 80, 559, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 15 },
        { 524, 105, 559, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 15 },
        { 559, 80, 594, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 16 },
        { 559, 105, 594, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 16 },
        { 594, 80, 629, 100, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 17 },
        { 594, 105, 629, 125, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 17 },
        { 34, 155, 69, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 18 },
        { 34, 180, 69, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 18 },
        { 69, 155, 104, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 19 },
        { 69, 180, 104, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 19 },
        { 104, 155, 139, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 20 },
        { 104, 180, 139, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 20 },
        { 139, 155, 174, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 21 },
        { 139, 180, 174, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 21 },
        { 174, 155, 209, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 22 },
        { 174, 180, 209, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 22 },
        { 209, 155, 244, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 23 },
        { 209, 180, 244, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 23 },
        { 244, 155, 279, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 24 },
        { 244, 180, 279, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 24 },
        { 279, 155, 314, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 25 },
        { 279, 180, 314, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 25 },
        { 314, 155, 349, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 26 },
        { 314, 180, 349, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 26 },
        { 349, 155, 384, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 27 },
        { 349, 180, 384, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 27 },
        { 384, 155, 419, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 28 },
        { 384, 180, 419, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 28 },
        { 419, 155, 454, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 29 },
        { 419, 180, 454, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 29 },
        { 454, 155, 489, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 30 },
        { 454, 180, 489, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 30 },
        { 489, 155, 524, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 31 },
        { 489, 180, 524, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 31 },
        { 524, 155, 559, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 32 },
        { 524, 180, 559, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 32 },
        { 559, 155, 594, 175, editor_trade_price_button, button_none, 0, 0, 0, 1, 0, 33 },
        { 559, 180, 594, 200, editor_trade_price_button, button_none, 0, 0, 0, 1, 1, 33 },
        { 450, 205, 570, 240, editor_trade_prices_reset, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .editor_trade_prices_end = 0,
    .empire_trade_route = {
        { 50, 68, 450, 91, empire_button_open_trade_route, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .empire_trade_route_end = 0,
    .empire_editor_city = {
        { 170, 2, 350, 22, empire_editor_city_type, button_none, 0, 0, 0, 3, 0, 0 },
        { 370, 2, 550, 22, empire_editor_city_name, button_none, 0, 0, 0, 3, 0, 0 },
        { 70, 2, 170, 22, empire_editor_city_text_align, button_none, 0, 0, 0, 1, 0, 0 },
        { 230, 27, 280, 47, empire_editor_city_route, button_none, 0, 0, 0, 3, 0, 0 },
        { 300, 27, 400, 47, empire_editor_trade_amounts, button_none, 0, 0, 0, 1, 0, 0 },
        { 480, 27, 540, 47, empire_editor_city_cost, button_none, 0, 0, 0, 3, 0, 0 },
        { 170, 30, 350, 50, empire_editor_region_name, button_none, 0, 0, 0, 3, 0, 0 }
    },
    .empire_editor_city_end = 0,
    .empire_editor_battle = {
        { 300, 46, 350, 66, empire_editor_battle_path, button_none, 0, 0, 0, 3, 0, 0 },
        { 300, 76, 350, 96, empire_editor_battle_order, button_none, 0, 0, 0, 3, 0, 0 }
    },
    .empire_editor_our_city_sold = {
        { 175, 52, 201, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 0, 0 },
        { 202, 52, 228, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 1, 0 },
        { 229, 52, 255, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 2, 0 },
        { 256, 52, 282, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 3, 0 },
        { 283, 52, 309, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 4, 0 },
        { 310, 52, 336, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 5, 0 },
        { 337, 52, 363, 78, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 6, 0 },
        { 175, 80, 201, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 7, 0 },
        { 202, 80, 228, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 8, 0 },
        { 229, 80, 255, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 9, 0 },
        { 256, 80, 282, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 10, 0 },
        { 283, 80, 309, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 11, 0 },
        { 310, 80, 336, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 12, 0 },
        { 337, 80, 363, 106, empire_editor_goods_slot, button_none, 0, 0, 0, 3, 13, 0 }
    },
    .empire_editor_trade_city_amounts = {
        { 300, 52, 326, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 0, 0 },
        { 330, 52, 356, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 1, 0 },
        { 360, 52, 386, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 2, 0 },
        { 390, 52, 416, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 3, 0 },
        { 420, 52, 446, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 4, 0 },
        { 450, 52, 476, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 5, 0 },
        { 480, 52, 506, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 6, 0 },
        { 510, 52, 536, 78, empire_editor_goods_sold_amount_slot, button_none, 0, 0, 0, 3, 7, 0 },
        { 300, 80, 326, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 0, 0 },
        { 330, 80, 356, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 1, 0 },
        { 360, 80, 386, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 2, 0 },
        { 390, 80, 416, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 3, 0 },
        { 420, 80, 446, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 4, 0 },
        { 450, 80, 476, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 5, 0 },
        { 480, 80, 506, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 6, 0 },
        { 510, 80, 536, 106, empire_editor_goods_bought_amount_slot, button_none, 0, 0, 0, 3, 7, 0 }
    },
    .numeric_input = {
        { 5, 35, 30, 60, numeric_input_digit, button_none, 0, 0, 0, 3, 1, 0 },
        { 35, 35, 60, 60, numeric_input_digit, button_none, 0, 0, 0, 3, 2, 0 },
        { 65, 35, 90, 60, numeric_input_digit, button_none, 0, 0, 0, 3, 3, 0 },
        { 5, 65, 30, 90, numeric_input_digit, button_none, 0, 0, 0, 3, 4, 0 },
        { 35, 65, 60, 90, numeric_input_digit, button_none, 0, 0, 0, 3, 5, 0 },
        { 65, 65, 90, 90, numeric_input_digit, button_none, 0, 0, 0, 3, 6, 0 },
        { 5, 95, 30, 120, numeric_input_digit, button_none, 0, 0, 0, 3, 7, 0 },
        { 35, 95, 60, 120, numeric_input_digit, button_none, 0, 0, 0, 3, 8, 0 },
        { 65, 95, 90, 120, numeric_input_digit, button_none, 0, 0, 0, 3, 9, 0 },
        { 5, 125, 30, 150, numeric_input_digit, button_none, 0, 0, 0, 3, 0, 0 },
        { 35, 125, 90, 150, numeric_input_ok, button_none, 0, 0, 0, 3, 1, 0 },
        { 5, 155, 120, 180, numeric_input_cancel, button_none, 0, 0, 0, 3, 1, 0 }
    }
};

name_list_button_table name_list_buttons = {
    .buttons = {
        { 5, 14, 101, 32, button_none, button_none, 0, 0, 0, 3, 0, 0 },
        { 5, 34, 101, 52, name_list_choose, button_none, 0, 0, 0, 3, 1, 0 },
        { 5, 54, 101, 72, name_list_choose, button_none, 0, 0, 0, 3, 2, 0 },
        { 5, 74, 101, 92, name_list_choose, button_none, 0, 0, 0, 3, 3, 0 },
        { 5, 94, 101, 112, name_list_choose, button_none, 0, 0, 0, 3, 4, 0 },
        { 5, 114, 101, 132, name_list_choose, button_none, 0, 0, 0, 3, 5, 0 },
        { 5, 134, 101, 152, name_list_choose, button_none, 0, 0, 0, 3, 6, 0 },
        { 5, 154, 101, 172, name_list_choose, button_none, 0, 0, 0, 3, 7, 0 },
        { 5, 174, 101, 192, name_list_choose, button_none, 0, 0, 0, 3, 8, 0 },
        { 5, 194, 101, 212, name_list_choose, button_none, 0, 0, 0, 3, 9, 0 },
        { 5, 214, 101, 232, name_list_choose, button_none, 0, 0, 0, 3, 10, 0 },
        { 5, 234, 101, 252, name_list_choose, button_none, 0, 0, 0, 3, 11, 0 },
        { 5, 254, 101, 272, name_list_choose, button_none, 0, 0, 0, 3, 12, 0 },
        { 5, 274, 101, 292, name_list_choose, button_none, 0, 0, 0, 3, 13, 0 },
        { 5, 294, 101, 312, name_list_choose, button_none, 0, 0, 0, 3, 14, 0 },
        { 5, 314, 101, 332, name_list_choose, button_none, 0, 0, 0, 3, 15, 0 },
        { 5, 334, 101, 352, name_list_choose, button_none, 0, 0, 0, 3, 16, 0 },
        { 5, 354, 101, 372, name_list_choose, button_none, 0, 0, 0, 3, 17, 0 },
        { 5, 374, 101, 392, name_list_choose, button_none, 0, 0, 0, 3, 18, 0 },
        { 5, 394, 101, 412, name_list_choose, button_none, 0, 0, 0, 3, 19, 0 },
        { 5, 414, 101, 432, name_list_choose, button_none, 0, 0, 0, 3, 20, 0 },
        { 230, 14, 326, 32, button_none, button_none, 0, 0, 0, 3, 21, 0 },
        { 230, 34, 326, 52, name_list_choose, button_none, 0, 0, 0, 3, 22, 0 },
        { 230, 54, 326, 72, name_list_choose, button_none, 0, 0, 0, 3, 23, 0 },
        { 230, 74, 326, 92, name_list_choose, button_none, 0, 0, 0, 3, 24, 0 },
        { 230, 94, 326, 112, name_list_choose, button_none, 0, 0, 0, 3, 25, 0 },
        { 230, 114, 326, 132, name_list_choose, button_none, 0, 0, 0, 3, 26, 0 },
        { 230, 134, 326, 152, name_list_choose, button_none, 0, 0, 0, 3, 27, 0 },
        { 230, 154, 326, 172, name_list_choose, button_none, 0, 0, 0, 3, 28, 0 },
        { 230, 174, 326, 192, name_list_choose, button_none, 0, 0, 0, 3, 29, 0 },
        { 230, 194, 326, 212, name_list_choose, button_none, 0, 0, 0, 3, 30, 0 },
        { 230, 214, 326, 232, name_list_choose, button_none, 0, 0, 0, 3, 31, 0 },
        { 230, 234, 326, 252, name_list_choose, button_none, 0, 0, 0, 3, 32, 0 },
        { 230, 254, 326, 272, name_list_choose, button_none, 0, 0, 0, 3, 33, 0 },
        { 230, 274, 326, 292, name_list_choose, button_none, 0, 0, 0, 3, 34, 0 },
        { 230, 294, 326, 312, name_list_choose, button_none, 0, 0, 0, 3, 35, 0 },
        { 230, 314, 326, 332, name_list_choose, button_none, 0, 0, 0, 3, 36, 0 },
        { 230, 334, 326, 352, name_list_choose, button_none, 0, 0, 0, 3, 37, 0 },
        { 230, 354, 326, 372, name_list_choose, button_none, 0, 0, 0, 3, 38, 0 },
        { 230, 374, 326, 392, name_list_choose, button_none, 0, 0, 0, 3, 39, 0 },
        { 230, 394, 326, 412, name_list_choose, button_none, 0, 0, 0, 3, 40, 0 },
        { 230, 414, 326, 432, name_list_choose, button_none, 0, 0, 0, 3, 41, 0 },
        { 325, 414, 400, 432, window_go_cheat_input, button_none, 0, 0, 0, 3, 0, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_delete_read = {
    .buttons = {
        { 0, 0, 280, 30, message_list_delete_read, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

message_list_button_tables buttons_rows = {
    .rows = {
        { 0, 0, 416, 18, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 0, 0 },
        { 0, 20, 416, 38, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 1, 0 },
        { 0, 40, 416, 58, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 2, 0 },
        { 0, 60, 416, 78, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 3, 0 },
        { 0, 80, 416, 98, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 4, 0 },
        { 0, 100, 416, 118, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 5, 0 },
        { 0, 120, 416, 138, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 6, 0 },
        { 0, 140, 416, 158, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 7, 0 },
        { 0, 160, 416, 178, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 8, 0 },
        { 0, 180, 416, 198, message_list_button_message, message_list_delete_message, 0, 0, 0, 3, 9, 0 },
        { 12, 1, 52, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 1, 0 },
        { 60, 1, 100, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 2, 0 },
        { 108, 1, 148, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 4, 0 },
        { 156, 1, 196, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 5, 0 },
        { 204, 1, 244, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 6, 0 },
        { 252, 1, 292, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 7, 0 },
        { 300, 1, 340, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 8, 0 },
        { 348, 1, 388, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 9, 0 },
        { 396, 1, 436, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 10, 0 },
        { 444, 1, 484, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 11, 0 },
        { 492, 1, 532, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 12, 0 },
        { 540, 1, 580, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 13, 0 },
        { 588, 1, 628, 41, button_advisor_bar, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .rows_end = 0,
    .labor_advisor = {
        { 40, 77, 600, 99, button_labor_priority_open, button_none, 0, 0, 0, 1, 0, 0 },
        { 40, 102, 600, 124, button_labor_priority_open, button_none, 0, 0, 0, 1, 1, 0 },
        { 40, 127, 600, 149, button_labor_priority_open, button_none, 0, 0, 0, 1, 2, 0 },
        { 40, 152, 600, 174, button_labor_priority_open, button_none, 0, 0, 0, 1, 3, 0 },
        { 40, 177, 600, 199, button_labor_priority_open, button_none, 0, 0, 0, 1, 4, 0 },
        { 40, 202, 600, 224, button_labor_priority_open, button_none, 0, 0, 0, 1, 5, 0 },
        { 40, 227, 600, 249, button_labor_priority_open, button_none, 0, 0, 0, 1, 6, 0 },
        { 40, 252, 600, 274, button_labor_priority_open, button_none, 0, 0, 0, 1, 7, 0 },
        { 40, 277, 600, 299, button_labor_priority_open, button_none, 0, 0, 0, 1, 8, 0 }
    },
    .labor_advisor_end = 0,
    .labor_priority = {
        { 180, 256, 460, 281, button_labor_priority_set, button_none, 0, 0, 0, 1, 0, 0 },
        { 178, 221, 205, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 1, 0 },
        { 210, 221, 237, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 2, 0 },
        { 242, 221, 269, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 3, 0 },
        { 274, 221, 301, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 4, 0 },
        { 306, 221, 333, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 5, 0 },
        { 338, 221, 365, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 6, 0 },
        { 370, 221, 397, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 7, 0 },
        { 402, 221, 429, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 8, 0 },
        { 434, 221, 461, 248, button_labor_priority_set, button_none, 0, 0, 0, 1, 9, 0 }
    },
    .military_advisor = {
        { 440, 83, 470, 113, button_military_go_to_legion, button_none, 0, 0, 0, 1, 1, 0 },
        { 500, 83, 530, 113, button_military_return_to_fort, button_none, 0, 0, 0, 1, 1, 0 },
        { 560, 83, 590, 113, button_military_empire_service, button_none, 0, 0, 0, 1, 1, 0 },
        { 440, 127, 470, 157, button_military_go_to_legion, button_none, 0, 0, 0, 1, 2, 0 },
        { 500, 127, 530, 157, button_military_return_to_fort, button_none, 0, 0, 0, 1, 2, 0 },
        { 560, 127, 590, 157, button_military_empire_service, button_none, 0, 0, 0, 1, 2, 0 },
        { 440, 171, 470, 201, button_military_go_to_legion, button_none, 0, 0, 0, 1, 3, 0 },
        { 500, 171, 530, 201, button_military_return_to_fort, button_none, 0, 0, 0, 1, 3, 0 },
        { 560, 171, 590, 201, button_military_empire_service, button_none, 0, 0, 0, 1, 3, 0 },
        { 440, 215, 470, 245, button_military_go_to_legion, button_none, 0, 0, 0, 1, 4, 0 },
        { 500, 215, 530, 245, button_military_return_to_fort, button_none, 0, 0, 0, 1, 4, 0 },
        { 560, 215, 590, 245, button_military_empire_service, button_none, 0, 0, 0, 1, 4, 0 },
        { 440, 259, 470, 289, button_military_go_to_legion, button_none, 0, 0, 0, 1, 5, 0 },
        { 500, 259, 530, 289, button_military_return_to_fort, button_none, 0, 0, 0, 1, 5, 0 },
        { 560, 259, 590, 289, button_military_empire_service, button_none, 0, 0, 0, 1, 5, 0 },
        { 440, 303, 470, 333, button_military_go_to_legion, button_none, 0, 0, 0, 1, 6, 0 },
        { 500, 303, 530, 333, button_military_return_to_fort, button_none, 0, 0, 0, 1, 6, 0 },
        { 560, 303, 590, 333, button_military_empire_service, button_none, 0, 0, 0, 1, 6, 0 },
        { 532, 347, 612, 395, window_advisors_show_checked, button_none, 0, 0, 0, 1, 3, 0 },
        { 545, 35, 620, 70, button_military_send_help, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .fleet_advisor = {
        { 440, 83, 470, 113, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 1, 0 },
        { 500, 83, 530, 113, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 1, 0 },
        { 560, 83, 590, 113, button_fleet_empire_service, button_none, 0, 0, 0, 1, 1, 0 },
        { 440, 127, 470, 157, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 2, 0 },
        { 500, 127, 530, 157, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 2, 0 },
        { 560, 127, 590, 157, button_fleet_empire_service, button_none, 0, 0, 0, 1, 2, 0 },
        { 440, 171, 470, 201, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 3, 0 },
        { 500, 171, 530, 201, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 3, 0 },
        { 560, 171, 590, 201, button_fleet_empire_service, button_none, 0, 0, 0, 1, 3, 0 },
        { 440, 215, 470, 245, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 4, 0 },
        { 500, 215, 530, 245, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 4, 0 },
        { 560, 215, 590, 245, button_fleet_empire_service, button_none, 0, 0, 0, 1, 4, 0 },
        { 440, 259, 470, 289, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 5, 0 },
        { 500, 259, 530, 289, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 5, 0 },
        { 560, 259, 590, 289, button_fleet_empire_service, button_none, 0, 0, 0, 1, 5, 0 },
        { 440, 303, 470, 333, button_fleet_go_to_ship, button_none, 0, 0, 0, 1, 6, 0 },
        { 500, 303, 530, 333, button_fleet_return_to_dock, button_none, 0, 0, 0, 1, 6, 0 },
        { 560, 303, 590, 333, button_fleet_empire_service, button_none, 0, 0, 0, 1, 6, 0 },
        { 532, 347, 612, 395, window_advisors_show_checked, button_none, 0, 0, 0, 1, 2, 0 },
        { 545, 35, 620, 70, button_fleet_send_help, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .population_advisor = {
        { 503, 61, 607, 116, button_population_graph, button_none, 0, 0, 0, 1, 0, 0 },
        { 503, 161, 607, 216, button_population_graph, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .religion_festival = {
        { 102, 348, 402, 368, button_hold_festival, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

festival_button_tables festival_square_button = {
    .festival_square = {
        { 82, 154, 382, 174, button_hold_festival, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .festival_square_end = 0,
    .festival_gods = {
        { 70, 96, 150, 186, button_festival_god, button_none, 0, 0, 0, 1, 0, 0 },
        { 170, 96, 250, 186, button_festival_god, button_none, 0, 0, 0, 1, 1, 0 },
        { 270, 96, 350, 186, button_festival_god, button_none, 0, 0, 0, 1, 2, 0 },
        { 370, 96, 450, 186, button_festival_god, button_none, 0, 0, 0, 1, 3, 0 },
        { 470, 96, 550, 186, button_festival_god, button_none, 0, 0, 0, 1, 4, 0 },
        { 102, 216, 532, 242, button_festival_size, button_none, 0, 0, 0, 1, 1, 0 },
        { 102, 246, 532, 272, button_festival_size, button_none, 0, 0, 0, 1, 2, 0 },
        { 102, 276, 532, 302, button_festival_size, button_none, 0, 0, 0, 1, 3, 0 }
    },
    .hold_festival_gods = {
        { 70, 96, 150, 186, button_festival_god, button_none, 0, 0, 0, 1, 0, 0 },
        { 170, 96, 250, 186, button_festival_god, button_none, 0, 0, 0, 1, 1, 0 },
        { 270, 96, 350, 186, button_festival_god, button_none, 0, 0, 0, 1, 2, 0 },
        { 370, 96, 450, 186, button_festival_god, button_none, 0, 0, 0, 1, 3, 0 },
        { 470, 96, 550, 186, button_festival_god, button_none, 0, 0, 0, 1, 4, 0 },
        { 102, 216, 532, 242, button_festival_size, button_none, 0, 0, 0, 1, 1, 0 },
        { 102, 246, 532, 272, button_festival_size, button_none, 0, 0, 0, 1, 2, 0 },
        { 102, 276, 532, 302, button_festival_size, button_none, 0, 0, 0, 1, 3, 0 }
    },
    .kingdom_advisor = {
        { 320, 353, 570, 373, button_donate_to_city, button_none, 0, 0, 0, 1, 0, 0 },
        { 70, 393, 570, 413, button_set_salary, button_none, 0, 0, 0, 1, 0, 0 },
        { 320, 332, 570, 352, button_send_gift, button_none, 0, 0, 0, 1, 0, 0 },
        { 38, 96, 598, 136, button_request, button_none, 0, 0, 0, 1, 0, 0 },
        { 38, 138, 598, 178, button_request, button_none, 0, 0, 0, 1, 1, 0 },
        { 38, 180, 598, 220, button_request, button_none, 0, 0, 0, 1, 2, 0 },
        { 38, 222, 598, 262, button_request, button_none, 0, 0, 0, 1, 3, 0 },
        { 38, 264, 598, 304, button_request, button_none, 0, 0, 0, 1, 4, 0 }
    }
};

mansion_button_tables mansion_salary_button = {
    .mansion_salary = {
        { 40, 61, 365, 86, button_set_salary, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .mansion_salary_end = 0,
    .salaries = {
        { 240, 395, 400, 415, button_salary_cancel, button_none, 0, 0, 0, 1, 0, 0 },
        { 144, 85, 432, 105, button_salary_rank, button_none, 0, 0, 0, 1, 0, 0 },
        { 144, 105, 432, 125, button_salary_rank, button_none, 0, 0, 0, 1, 1, 0 },
        { 144, 125, 432, 145, button_salary_rank, button_none, 0, 0, 0, 1, 2, 0 },
        { 144, 145, 432, 165, button_salary_rank, button_none, 0, 0, 0, 1, 3, 0 },
        { 144, 165, 432, 185, button_salary_rank, button_none, 0, 0, 0, 1, 4, 0 },
        { 144, 185, 432, 205, button_salary_rank, button_none, 0, 0, 0, 1, 5, 0 },
        { 144, 205, 432, 225, button_salary_rank, button_none, 0, 0, 0, 1, 6, 0 },
        { 144, 225, 432, 245, button_salary_rank, button_none, 0, 0, 0, 1, 7, 0 },
        { 144, 245, 432, 265, button_salary_rank, button_none, 0, 0, 0, 1, 8, 0 },
        { 144, 265, 432, 285, button_salary_rank, button_none, 0, 0, 0, 1, 9, 0 },
        { 144, 285, 432, 305, button_salary_rank, button_none, 0, 0, 0, 1, 10, 0 }
    },
    .donations = {
        { 336, 283, 496, 303, window_resource_settings_back, button_none, 0, 0, 0, 1, 0, 0 },
        { 144, 283, 304, 303, button_donate_give, button_none, 0, 0, 0, 1, 0, 0 },
        { 128, 216, 192, 236, button_donate_amount, button_none, 0, 0, 0, 1, 0, 0 },
        { 208, 216, 272, 236, button_donate_amount, button_none, 0, 0, 0, 1, 1, 0 },
        { 288, 216, 352, 236, button_donate_amount, button_none, 0, 0, 0, 1, 2, 0 },
        { 368, 216, 432, 236, button_donate_amount, button_none, 0, 0, 0, 1, 3, 0 },
        { 448, 216, 512, 236, button_donate_amount, button_none, 0, 0, 0, 1, 4, 0 }
    },
    .donations_end = 0,
    .gifts = {
        { 208, 213, 528, 233, button_gift_select, button_none, 0, 0, 0, 1, 1, 0 },
        { 208, 233, 528, 253, button_gift_select, button_none, 0, 0, 0, 1, 2, 0 },
        { 208, 253, 528, 273, button_gift_select, button_none, 0, 0, 0, 1, 3, 0 },
        { 128, 336, 368, 356, button_gift_send, button_none, 0, 0, 0, 1, 0, 0 },
        { 400, 336, 560, 356, window_resource_settings_back, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .gifts_end = 0,
    .burial_provisions = {
        { 328, 213, 528, 233, burial_provisions_all, button_none, 0, 0, 0, 1, 1, 0 },
        { 128, 336, 368, 356, burial_provisions_send, button_none, 0, 0, 0, 1, 0, 0 },
        { 400, 336, 560, 356, burial_provisions_back, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .burial_provisions_end = 0,
    .burial_provision_arrows = {
        { 0, 0, 16, 24, burial_provisions_more, 0, 0, 0, 4, 0, 1 },
        { 0, 24, 18, 24, burial_provisions_less, 0, 0, 0, 4, 1, 1 }
    },
    .monuments_advisor = {
        { 38, 58, 598, 100, button_monument_resource, button_none, 0, 0, 0, 1, 0, 0 },
        { 38, 100, 598, 142, button_monument_resource, button_none, 0, 0, 0, 1, 1, 0 },
        { 38, 142, 598, 184, button_monument_resource, button_none, 0, 0, 0, 1, 2, 0 },
        { 38, 184, 598, 226, button_monument_resource, button_none, 0, 0, 0, 1, 3, 0 },
        { 38, 226, 598, 268, button_monument_resource, button_none, 0, 0, 0, 1, 4, 0 },
        { 38, 278, 598, 320, button_monument_go_to, button_none, 0, 0, 0, 1, 0, 0 },
        { 38, 320, 598, 362, button_monument_go_to, button_none, 0, 0, 0, 1, 1, 0 },
        { 38, 362, 598, 404, button_monument_go_to, button_none, 0, 0, 0, 1, 2, 0 }
    }
};

generic_button chief_advisor_rows[CHIEF_ADVISOR_ROWS] = { { 60, 66, 588, 86, chief_advisor_button_row, button_none, 0, 0, 0, 1, 0,
                                                 0 },
    { 60, 86, 588, 106, chief_advisor_button_row, button_none, 0, 0, 0, 1, 1, 0 },
    { 60, 106, 588, 126, chief_advisor_button_row, button_none, 0, 0, 0, 1, 2, 0 },
    { 60, 126, 588, 146, chief_advisor_button_row, button_none, 0, 0, 0, 1, 3, 0 },
    { 60, 146, 588, 166, chief_advisor_button_row, button_none, 0, 0, 0, 1, 4, 0 },
    { 60, 166, 588, 186, chief_advisor_button_row, button_none, 0, 0, 0, 1, 5, 0 },
    { 60, 186, 588, 206, chief_advisor_button_row, button_none, 0, 0, 0, 1, 6, 0 },
    { 60, 206, 588, 226, chief_advisor_button_row, button_none, 0, 0, 0, 1, 7, 0 },
    { 60, 226, 588, 246, chief_advisor_button_row, button_none, 0, 0, 0, 1, 8, 0 },
    { 60, 246, 588, 266, chief_advisor_button_row, button_none, 0, 0, 0, 1, 9, 0 },
    { 60, 266, 588, 286, chief_advisor_button_row, button_none, 0, 0, 0, 1, 10, 0 },
    { 60, 286, 588, 306, chief_advisor_button_row, button_none, 0, 0, 0, 1, 11, 0 } };

chief_nile_and_god_buttons row_nile2 = {
    .chief_nile_row = {
        { 60, 306, 588, 326, chief_advisor_button_row, button_none, 0, 0, 0, 1, 11, 0 }
    },
    .chief_nile_row_end = 0,
    .religion_gods = {
        { 40, 67, 568, 106, window_go_god_description, button_none, 0, 0, 0, 1, 0, 0 },
        { 40, 107, 568, 146, window_go_god_description, button_none, 0, 0, 0, 1, 1, 0 },
        { 40, 147, 568, 186, window_go_god_description, button_none, 0, 0, 0, 1, 2, 0 },
        { 40, 187, 568, 226, window_go_god_description, button_none, 0, 0, 0, 1, 3, 0 },
        { 40, 227, 568, 266, window_go_god_description, button_none, 0, 0, 0, 1, 4, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_house_ventures = {
    .buttons = {
        { 0, 0, 240, 20, building_info_toggle_details, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_granary_orders = {
    .buttons = {
        { 0, 0, 272, 20, building_info_show_orders, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_bazaar_orders = {
    .buttons = {
        { 0, 0, 272, 20, building_info_show_orders, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_storage_orders = {
    .buttons = {
        { 0, 0, 272, 20, building_info_show_orders, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

family_name_list_button_table family_name_list_button = {
    .buttons = {
        { 0, 0, 25, 25, window_go_name_list, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0,
    .unused_arrows = {
        { 0, 0, 18, 24, city_toggle_resource_settings_step, 0, 0, 0, 4, 1, 1 },
        { 0, 0, 18, 24, building_info_toggle_dc, 0, 0, 0, 4, 1, 1 }
    }
};

arrow_button storage_orders_scroll_down[1] = { { 0, 0, 18, 24, building_orders_scroll_arrow, 0, 0, 0, 4, 1, 1 } };

arrow_button storage_orders_scroll_up[1] = { { 24, 0, 16, 24, building_orders_scroll_arrow, 0, 0, 0, 4, 0, 1 } };

arrow_button trade_advisor_scroll_down[1] = { { 0, 0, 18, 24, city_list_scroll_arrow, 0, 0, 0, 4, 1, 1 } };

arrow_button trade_advisor_scroll_up[1] = { { 24, 0, 16, 24, city_list_scroll_arrow, 0, 0, 0, 4, 0, 1 } };

single_generic_button buttons_back_orders = {
    .buttons = {
        { 0, 0, 304, 20, storage_orders_accept_none, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_storage_back = {
    .buttons = {
        { 0, 20, 304, 40, window_building_info_storage_special_button, button_none, 0, 0, 0, 1, 2, 0 }
    },
    .list_end = 0
};

figure_list_button_table buttons_figure_list = {
    .buttons = {
        { 26, 46, 76, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 0, 0 },
        { 86, 46, 136, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 1, 0 },
        { 146, 46, 196, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 2, 0 },
        { 206, 46, 256, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 3, 0 },
        { 266, 46, 316, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 4, 0 },
        { 326, 46, 376, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 5, 0 },
        { 386, 46, 436, 96, window_building_info_select_figure, button_none, 0, 0, 0, 1, 6, 0 }
    },
    .list_end = 0
};

figure_phrase_button_tables buttons_figure_phrase = {
    .phrase = {
        { 0, 0, 150, 25, window_figure_info_button_city_problem, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .phrase_end = 0,
    .editor_settings = {
        { 250, 80, 450, 110, editor_settings_start_year, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 120, 450, 150, editor_settings_funds, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 160, 450, 190, editor_settings_bailout, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 200, 450, 230, editor_settings_milestone, button_none, 0, 0, 0, 3, 0, 0 },
        { 250, 240, 450, 270, editor_settings_milestone, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 280, 450, 310, editor_settings_milestone, button_none, 0, 0, 0, 3, 2, 0 },
        { 250, 40, 450, 70, editor_settings_empire_map, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 320, 450, 350, editor_settings_interest, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 360, 450, 390, editor_settings_pharaoh_name, button_none, 0, 0, 0, 3, 1, 0 },
        { 250, 400, 450, 430, editor_settings_player_title, button_none, 0, 0, 0, 3, 1, 0 },
        { 0 },
        { 0 }
    },
    .editor_flood = {
        { 220, 75, 420, 105, editor_flood_season_next, button_none, 0, 0, 0, 1, 1, 0 },
        { 220, 130, 420, 160, editor_flood_duration_next, button_none, 0, 0, 0, 1, 1, 0 },
        { 220, 185, 420, 215, editor_flood_quality_next, button_none, 0, 0, 0, 1, 1, 0 }
    },
    .list_end = 0
};

generic_button select_list_extra_buttons[2] = {
    { 0, 0, 200, 30, select_list_pharaoh_names_page_2, button_none, 0, 0, 0, 3, 1, 0 },
    { 0, 0, 200, 30, select_list_pharaoh_names_page_1, button_none, 0, 0, 0, 3, 1, 0 },
};

editor_allowed_building_button_table editor_allowed_buildings_buttons = {
    .buttons = {
        { 9, 50, 199, 68, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 2, 0 },
        { 9, 50, 199, 88, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 3, 0 },
        { 9, 70, 199, 108, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 4, 0 },
        { 9, 90, 199, 128, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 5, 0 },
        { 9, 110, 199, 148, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 6, 0 },
        { 9, 130, 199, 168, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 7, 0 },
        { 9, 150, 199, 188, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 8, 0 },
        { 9, 170, 199, 208, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 9, 0 },
        { 9, 190, 199, 228, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 10, 0 },
        { 9, 210, 199, 248, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 11, 0 },
        { 9, 230, 199, 268, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 12, 0 },
        { 9, 250, 199, 288, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 13, 0 },
        { 9, 270, 199, 308, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 14, 0 },
        { 9, 290, 199, 328, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 15, 0 },
        { 9, 310, 199, 348, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 16, 0 },
        { 9, 330, 199, 368, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 17, 0 },
        { 208, 50, 398, 68, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 18, 0 },
        { 208, 70, 398, 88, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 19, 0 },
        { 208, 90, 398, 108, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 20, 0 },
        { 208, 110, 398, 128, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 21, 0 },
        { 208, 130, 398, 148, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 22, 0 },
        { 208, 150, 398, 168, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 23, 0 },
        { 208, 170, 398, 188, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 24, 0 },
        { 208, 190, 398, 208, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 25, 0 },
        { 208, 210, 398, 228, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 26, 0 },
        { 208, 230, 398, 248, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 27, 0 },
        { 208, 250, 398, 268, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 28, 0 },
        { 208, 270, 398, 288, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 29, 0 },
        { 208, 290, 398, 308, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 30, 0 },
        { 208, 310, 398, 328, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 31, 0 },
        { 208, 330, 398, 348, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 32, 0 },
        { 208, 350, 398, 368, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 33, 0 },
        { 407, 50, 597, 68, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 34, 0 },
        { 407, 70, 597, 88, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 35, 0 },
        { 407, 90, 597, 108, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 36, 0 },
        { 407, 110, 597, 128, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 37, 0 },
        { 407, 130, 597, 148, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 38, 0 },
        { 407, 150, 597, 168, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 39, 0 },
        { 407, 170, 597, 188, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 40, 0 },
        { 407, 190, 597, 208, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 41, 0 },
        { 407, 210, 597, 228, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 42, 0 },
        { 407, 230, 597, 248, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 43, 0 },
        { 407, 250, 597, 268, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 44, 0 },
        { 407, 270, 597, 288, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 45, 0 },
        { 407, 290, 597, 308, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 46, 0 },
        { 407, 310, 597, 328, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 47, 0 },
        { 407, 330, 597, 348, editor_allowed_building_toggle, button_none, 0, 0, 0, 1, 48, 0 }
    },
    .list_end = 0
};

generic_button editor_win_criteria_buttons[18] = { { 300, 140, 380, 170, editor_criteria_goal_toggle,
                                                               button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 140, 580, 170, editor_criteria_goal, button_none, 0, 0, 0, 3, 0, 0 },
    { 300, 180, 380, 210, editor_criteria_goal_toggle, button_none, 0, 0, 0, 3, 1, 0 },
    { 400, 180, 580, 210, editor_criteria_goal, button_none, 0, 0, 0, 3, 1, 0 },
    { 300, 220, 380, 250, editor_criteria_goal_toggle, button_none, 0, 0, 0, 3, 2, 0 },
    { 400, 220, 580, 250, editor_open_monuments, button_none, 0, 0, 0, 3, 2, 0 },
    { 300, 260, 380, 290, editor_criteria_goal_toggle, button_none, 0, 0, 0, 3, 3, 0 },
    { 400, 260, 580, 290, editor_criteria_goal, button_none, 0, 0, 0, 3, 3, 0 },
    { 300, 300, 380, 330, editor_criteria_time_limit_toggle, button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 300, 580, 330, editor_criteria_time_limit_years, button_none, 0, 0, 0, 3, 0, 0 },
    { 300, 340, 380, 370, editor_criteria_survival_toggle, button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 340, 580, 370, editor_criteria_survival_years, button_none, 0, 0, 0, 3, 0, 0 },
    { 300, 380, 380, 410, editor_criteria_population_toggle, button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 380, 580, 410, editor_criteria_population, button_none, 0, 0, 0, 3, 0, 0 },
    { 300, 60, 380, 90, editor_criteria_open_play_toggle, button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 60, 580, 90, editor_criteria_open_play_id, button_none, 0, 0, 0, 3, 0, 0 },
    { 300, 100, 380, 130, editor_criteria_houses, button_none, 0, 0, 0, 3, 0, 0 },
    { 400, 100, 580, 130, editor_criteria_houses_level, button_none, 0, 0, 0, 3, 0, 0 } };

victory_dialog_button_table victory_dialog_buttons = {
    .buttons = {
        { 32, 112, 416, 132, window_go_city, button_none, 0, 0, 0, 3, 0, 0 },
        { 32, 144, 416, 164, victory_button_continue_years, button_none, 0, 0, 0, 1, 1, 0 },
        { 32, 176, 416, 196, victory_button_continue_years, button_none, 0, 0, 0, 1, 2, 0 }
    },
    .list_end = 0
};

defeat_dialog_button_tables defeat_dialog_buttons = {
    .menu = {
        { 64, 208, 480, 228, defeat_button_menu, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .menu_end = 0,
    .choices = {
        { 64, 208, 208, 228, defeat_button_menu, button_none, 0, 0, 0, 1, 0, 0 },
        { 352, 208, 496, 228, defeat_button_replay, button_none, 0, 0, 0, 1, 0, 0 },
        { 32, 208, 176, 228, defeat_button_easier, button_none, 0, 0, 0, 1, 0, 0 },
        { 208, 208, 352, 228, defeat_button_menu, button_none, 0, 0, 0, 1, 0, 0 },
        { 384, 208, 528, 228, defeat_button_replay, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

order_button_row_table buttons_legion = {
    .buttons = {
        { 19, 139, 103, 223, legion_order_button, button_none, 0, 0, 0, 1, 0, 0 },
        { 104, 139, 188, 223, legion_order_button, button_none, 0, 0, 0, 1, 1, 0 },
        { 189, 139, 273, 223, legion_order_button, button_none, 0, 0, 0, 1, 2, 0 },
        { 274, 139, 358, 223, legion_order_button, button_none, 0, 0, 0, 1, 3, 0 },
        { 359, 139, 443, 223, legion_return_button, button_none, 0, 0, 0, 1, 4, 0 }
    },
    .list_end = 0
};

single_generic_button buttons_legion_return = {
    .buttons = {
        { 0, 0, 320, 30, legion_order_button_toggle, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

order_button_row_table buttons_boat = {
    .buttons = {
        { 19, 139, 103, 223, transport_ship_button, button_none, 0, 0, 0, 1, 0, 0 },
        { 104, 139, 188, 223, transport_ship_button, button_none, 0, 0, 0, 1, 1, 0 },
        { 189, 139, 273, 223, transport_ship_button, button_none, 0, 0, 0, 1, 2, 0 },
        { 274, 139, 358, 223, transport_ship_button, button_none, 0, 0, 0, 1, 3, 0 },
        { 359, 139, 443, 223, transport_ship_order_button, button_none, 0, 0, 0, 1, 4, 0 }
    },
    .list_end = 0
};

ship_order_button_tables buttons_ship = {
    .orders = {
        { 19, 139, 103, 223, warship_button, button_none, 0, 0, 0, 1, 0, 0 },
        { 104, 139, 188, 223, warship_button, button_none, 0, 0, 0, 1, 1, 0 },
        { 189, 139, 273, 223, warship_button, button_none, 0, 0, 0, 1, 2, 0 },
        { 274, 139, 358, 223, warship_button, button_none, 0, 0, 0, 1, 3, 0 },
        { 359, 139, 443, 223, warship_order_button, button_none, 0, 0, 0, 1, 4, 0 }
    },
    .orders_end = 0,
    .display_mode_confirm = {
        { 160, 112, 320, 144, display_check_button_ok, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .list_end = 0
};

special_order_button_tables buttons_special_orders = {
    .resources = {
        { 0, 0, 165, 20, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 1, 0 },
        { 0, 20, 165, 40, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 2, 0 },
        { 0, 40, 165, 60, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 3, 0 },
        { 0, 60, 165, 80, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 4, 0 },
        { 0, 80, 165, 100, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 5, 0 },
        { 0, 100, 165, 120, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 6, 0 },
        { 0, 120, 165, 140, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 7, 0 },
        { 0, 140, 165, 160, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 8, 0 },
        { 0, 160, 165, 180, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 9, 0 },
        { 0, 180, 165, 200, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 10, 0 },
        { 0, 200, 165, 220, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 11, 0 },
        { 0, 220, 165, 240, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 12, 0 },
        { 0, 240, 165, 260, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 13, 0 },
        { 0, 260, 165, 280, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 14, 0 },
        { 0, 280, 165, 300, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 15, 0 },
        { 0, 300, 165, 320, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 16, 0 },
        { 0, 320, 165, 340, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 17, 0 },
        { 0, 340, 165, 360, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 18, 0 },
        { 0, 360, 165, 380, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 19, 0 },
        { 0, 380, 165, 400, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 20, 0 },
        { 0, 400, 165, 420, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 21, 0 },
        { 0, 420, 165, 440, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 22, 0 },
        { 0, 440, 165, 460, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 23, 0 },
        { 0, 460, 165, 480, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 24, 0 },
        { 0, 480, 165, 500, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 25, 0 },
        { 0, 500, 165, 520, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 26, 0 },
        { 0, 520, 165, 540, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 27, 0 },
        { 0, 540, 165, 560, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 28, 0 },
        { 0, 560, 165, 580, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 29, 0 },
        { 0, 580, 165, 600, storage_orders_toggle_resource, button_none, 0, 0, 0, 1, 30, 0 }
    },
    .blank_resources = {
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 }
    },
    .sidebar_problem = {
        { 86, 404, 114, 449, city_button_go_to_problem, button_none, 0, 0, 0, 4, 1, 0 }
    },
    .sidebar_problem_end = 0,
    .editor_gods = {
        { 100, 70, 200, 90, editor_gods_next_status, button_none, 0, 0, 0, 1, 0, 0 },
        { 100, 94, 200, 114, editor_gods_next_status, button_none, 0, 0, 0, 1, 1, 1 },
        { 100, 118, 200, 138, editor_gods_next_status, button_none, 0, 0, 0, 1, 2, 2 },
        { 100, 142, 200, 162, editor_gods_next_status, button_none, 0, 0, 0, 1, 3, 3 },
        { 100, 166, 200, 186, editor_gods_next_status, button_none, 0, 0, 0, 1, 4, 4 },
        { 100, 190, 200, 210, editor_gods_next_status, button_none, 0, 0, 0, 1, 5, 5 },
        { 100, 214, 200, 234, editor_gods_next_status, button_none, 0, 0, 0, 1, 6, 6 },
        { 100, 238, 200, 258, editor_gods_next_status, button_none, 0, 0, 0, 1, 7, 7 },
        { 100, 262, 200, 282, editor_gods_next_status, button_none, 0, 0, 0, 1, 8, 8 },
        { 100, 286, 200, 306, editor_gods_next_status, button_none, 0, 0, 0, 1, 9, 9 },
        { 205, 70, 300, 90, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 0, 0 },
        { 205, 94, 300, 114, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 1, 0 },
        { 205, 118, 300, 138, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 2, 0 },
        { 205, 142, 300, 162, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 3, 0 },
        { 205, 166, 300, 186, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 4, 0 },
        { 205, 190, 300, 210, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 5, 0 },
        { 205, 214, 300, 234, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 6, 0 },
        { 205, 238, 300, 258, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 7, 0 },
        { 205, 262, 300, 282, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 8, 0 },
        { 205, 286, 300, 306, editor_gods_toggle_temple_complex, button_none, 0, 0, 0, 1, 9, 0 }
    },
    .editor_monuments = {
        { 10, 70, 280, 90, editor_monument_select, button_none, 0, 0, 0, 3, 0, 0 },
        { 10, 94, 280, 114, editor_monument_select, button_none, 0, 0, 0, 3, 1, 1 },
        { 10, 118, 280, 138, editor_monument_select, button_none, 0, 0, 0, 3, 2, 2 },
        { 375, 25, 600, 50, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 1, 2 },
        { 375, 50, 600, 75, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 8, 2 },
        { 375, 75, 600, 100, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 10, 2 },
        { 375, 100, 600, 125, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 13, 2 },
        { 375, 125, 600, 150, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 15, 2 },
        { 375, 150, 600, 175, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 17, 2 },
        { 375, 175, 600, 200, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 18, 2 },
        { 375, 200, 600, 225, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 19, 2 },
        { 375, 225, 600, 250, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 20, 2 },
        { 375, 250, 600, 275, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 23, 2 },
        { 375, 275, 600, 300, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 24, 2 },
        { 375, 300, 600, 325, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 25, 2 },
        { 375, 325, 600, 350, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 26, 2 },
        { 375, 350, 600, 375, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 28, 2 },
        { 375, 375, 600, 400, editor_monument_goods_amount, button_none, 0, 0, 0, 3, 30, 2 },
        { 10, 22, 280, 42, editor_monument_animation_set, button_none, 0, 0, 0, 3, 0, 0 }
    },
    .editor_monuments_end = 0,
    .editor_temple_complex_arrows = {
        { 412, 46, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 0 },
        { 436, 46, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 0 },
        { 412, 70, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 1 },
        { 436, 70, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 1 },
        { 412, 94, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 2 },
        { 436, 94, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 2 },
        { 412, 118, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 3 },
        { 436, 118, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 3 },
        { 412, 142, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 4 },
        { 436, 142, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 4 },
        { 412, 166, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 5 },
        { 436, 166, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 5 },
        { 412, 190, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 6 },
        { 436, 190, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 6 },
        { 412, 214, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 7 },
        { 436, 214, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 7 },
        { 412, 238, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 8 },
        { 436, 238, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 8 },
        { 412, 262, 18, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 1, 9 },
        { 436, 262, 16, 24, editor_gods_toggle_temple_complex, 0, 0, 0, 4, 0, 9 }
    }
};

arrow_button empire_editor_arrow_buttons[7] = { { 8, 8, 16, 24, empire_editor_image_arrow, 0, 0, 0, 4, 1, 0 },
    { 8, 32, 18, 24, empire_editor_image_arrow, 0, 0, 0, 4, 0, 0 },
    { 32, 8, 25, 24, empire_editor_image_arrow, 0, 0, 0, 4, 2, 0 },
    { 32, 32, 25, 24, empire_editor_image_arrow, 0, 0, 0, 4, 3, 0 },
    { 32, 56, 25, 24, empire_editor_image_arrow, 0, 0, 0, 4, 4, 0 },
    { 240, 10, 18, 24, empire_editor_object_step, 0, 0, 0, 4, 1, 0 },
    { 270, 10, 16, 24, empire_editor_object_step, 0, 0, 0, 4, 0, 0 } };

arrow_button advisor_labor_arrow_buttons[2] = { { 158, 354, 18, 24, advisor_labor_wages_arrow, 0, 0, 0, 4, 1, 0 },
    { 182, 354, 16, 24, advisor_labor_wages_arrow, 0, 0, 0, 4, 0, 0 } };

arrow_button advisor_finance_tax_arrow_buttons[2] = { { 180, 70, 18, 24, advisor_finance_tax_arrow, 0, 0, 0, 4, 1, 0 },
    { 204, 70, 16, 24, advisor_finance_tax_arrow, 0, 0, 0, 4, 0, 0 } };

arrow_button donate_arrow_buttons[2] = { { 240, 242, 18, 24, advisor_donation_arrow, 0, 0, 0, 4, 1, 0 },
    { 264, 242, 16, 24, advisor_donation_arrow, 0, 0, 0, 4, 0, 0 } };

arrow_button speed_options_arrow_buttons[4] = { { 112, 100, 18, 24, settings_change_game_speed, 0, 0, 0, 4, 1, 0 },
    { 136, 100, 16, 24, settings_change_game_speed, 0, 0, 0, 4, 0, 0 },
    { 112, 136, 18, 24, speed_options_arrow_scroll, 0, 0, 0, 4, 1, 0 },
    { 136, 136, 16, 24, speed_options_arrow_scroll, 0, 0, 0, 4, 0, 0 } };

arrow_button sound_options_arrow_buttons[8] = { { 112, 100, 18, 24, sound_options_arrow_music, 0, 0, 0, 4, 1, 0 },
    { 136, 100, 16, 24, sound_options_arrow_music, 0, 0, 0, 4, 0, 0 },
    { 112, 130, 18, 24, sound_options_arrow_speech, 0, 0, 0, 4, 1, 0 },
    { 136, 130, 16, 24, sound_options_arrow_speech, 0, 0, 0, 4, 0, 0 },
    { 112, 160, 18, 24, sound_options_arrow_effects, 0, 0, 0, 4, 1, 0 },
    { 136, 160, 16, 24, sound_options_arrow_effects, 0, 0, 0, 4, 0, 0 },
    { 112, 190, 18, 24, sound_options_arrow_city, 0, 0, 0, 4, 1, 0 },
    { 136, 190, 16, 24, sound_options_arrow_city, 0, 0, 0, 4, 0, 0 } };

arrow_button editor_scenario_image_arrow_buttons[2] = { { 8, 344, 19, 24, editor_scenario_image_arrow, 0, 0, 0, 4, 1, 0 },
    { 32, 344, 21, 24, editor_scenario_image_arrow, 0, 0, 0, 4, 0, 0 } };

difficulty_arrow_table difficulty_arrows = {
    .arrows = {
        { 0, 54, 16, 24, difficulty_options_arrow, 0, 0, 0, 4, 0, 0 },
        { 24, 54, 18, 24, difficulty_options_arrow, 0, 0, 0, 4, 1, 0 }
    },
    .unused = { 0, 0, 0, 0, 0, 0 }
};

menu_bar_entry menu_bar_city[MENU_BAR_ENTRIES] = {
    { 0, 0, LANG_GROUP_MENU_FILE, 0, 6 },
    { 0, 0, LANG_GROUP_MENU_OPTIONS, 6, 8 },
    { 0, 0, LANG_GROUP_MENU_HELP, 14, 4 },
    { 0, 0, LANG_GROUP_MENU_OVERSEERS, 21, 13 },
};

menu_bar_entry menu_bar_editor[MENU_BAR_ENTRIES] = {
    { 0, 0, LANG_GROUP_EDITOR_MENU_FILE, 34, 4 },
    { 0, 0, LANG_GROUP_MENU_OPTIONS, 6, 4 },
    { 0, 0, LANG_GROUP_MENU_HELP, 18, 3 },
    { 0, 0, LANG_GROUP_EDITOR_MENU_RESETS, 38, 6 },
};

int32_t build_menu_types[1607] = { 12, 163, 164, 198, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 7, 8, 194, 115, 76, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 92, 180, 49, 46, 206, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 150, 96, 97, 48, 209, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 31, 30, 33, 32, 36, 34, 35, 226, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 167, 81, 55, 86, 184, 187, 188, 189, 77, 78, 79,
    138, 52, 91, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 113, 114, 110, 111, 203, 204, 74, 232, 233, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 176, 95, 57, 94, 112, 205, 182, 181, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 70, 72, 75, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 103, 105, 104, 196, 100, 101, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 109, 106, 107, 216, 221, 195, 162, 161, 217, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 177,
    179, 178, 231, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 61, 62, 63, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 66, 67, 68, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45, 44, 40, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 18, 19, 20, 34, 35, 36, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 140, 141, 142, 143, 144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 59, 200, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 168, 172, 201, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 169, 173, 202, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 82, 136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 38, 41, 42, 43, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 211, 212, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 322, 9, 306, 10, 274, 12, 258, 13, 226,
    15, 210, 16, 178, 18, 162, 19, 130, 21, 114, 22, 82, 24, 66, 25, 34, 27, 18, 28, -14, 30, -30, 31, -46, 32, -62, 33, -78, 34, -78, 35, -94, 36, -94, 37,
    -110, 38, -110, 39, 162, 19, 162, 19, 162, 19, 162, 19, 162, 19, 162, 19, 0, 31, 45, 1, 3, 5, 6, 7, 4, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 10, 43, 48, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 12, 13, 14, 15,
    16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 18, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 21, 23, 22, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 26, 27, 49, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 };

int32_t empire_map_width = 1200;

int32_t empire_map_height = 1600;

int32_t empire_border_x = 16;

int32_t empire_border_y = 16;

int32_t empire_border_bottom = 130;

int32_t empire_hovered_event_marker = -1;

high_score_button_table high_scores_buttons = {
    .buttons = {
        { 576, 96, 34, 34, 4, 365, 8, 0, high_scores_scroll, button_none, 1, 0, 0, 0, 0, 0 },
        { 576, 424, 34, 34, 4, 365, 12, 0, high_scores_scroll, button_none, 1, 0, 0, 0, 1, 0 }
    },
    .unused = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const char str_highscore_file[24] = "save\\highscore.jas";

phrase_button_table phrase_buttons = {
    .buttons = {
        { 38, 280, 168, 305, gift_button_refuse, button_none, 0, 0, 0, 1, 0, 0 },
        { 172, 280, 302, 305, gift_button_later, button_none, 0, 0, 0, 1, 0, 0 },
        { 306, 280, 436, 305, gift_button_accept, button_none, 0, 0, 0, 1, 0, 0 }
    },
    .building_info_tooltips = { 0x47, 0x47, 0x48, 0x48, 0x49, 0x4A, 0x46, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53 }
};

char money_format[4] = " Db";

char pct_postfix[4] = "%";

int32_t route_point_selected = -1;

int32_t boat_order_image_offsets[5] = { 0, 1, 2, 4, 5 };

void *tutorial_buttons[15] = {
    message_help.buttons, message_labor_advisor.buttons,
    message_military_advisor[0].buttons, message_military_advisor[1].buttons,
    message_imperial_advisor[0].buttons, message_imperial_advisor[1].buttons,
    message_trade_advisor.buttons, message_population_advisor.buttons,
    message_health_advisor[0].buttons, message_health_advisor[1].buttons,
    message_health_advisor[2].buttons, message_religion_advisor[0].buttons,
    message_religion_advisor[1].buttons, message_religion_advisor[2].buttons,
    message_religion_advisor[3].buttons
};

const char player_file_pattern[12] = "Save\\*.dat";

const int32_t large_label_frames[30] = { 0, 14, 10, 0, 14, 10, 1, 13, 9, 1, 13, 9, 2, 12, 8, 3, 11, 7, 4, 9, 5, 5, 7, 3, 6, 5, 1, 7, 4, 0 };

generic_button message_box_buttons[3] = { { 0, 0, 0, 0, window_message_box_button, button_none, 0, 0, 0, 1, 0,
                                                      0 },
    { 0, 0, 0, 0, window_message_box_button, button_none, 0, 0, 0, 1, 1, 0 },
    { 0, 0, 0, 0, window_message_box_button, button_none, 0, 0, 0, 1, 2, 0 } };

char money_postfix[4] = "Db";

char wages_rome_format[4] = " )";

char unemployment_format[4] = "%)";

char db_format[4] = "db";

generic_button trade_advisor_buttons[18] = { { 400, 398, 600, 421, button_trade_prices_mode_toggle, button_none, 0, 0,
                                                         0, 1, 1, 0 },
    { 150, 398, 350, 421, button_trade_exit, button_none, 0, 0, 0, 1, 1, 0 },
    { 506, 0, 606, 0, button_none, button_none, 0, 0, 0, 1, 0, 0 },
    { 26, 56, 506, 76, button_trade_resource, button_none, 0, 0, 0, 1, 0, 0 },
    { 26, 78, 506, 98, button_trade_resource, button_none, 0, 0, 0, 1, 1, 0 },
    { 26, 100, 506, 120, button_trade_resource, button_none, 0, 0, 0, 1, 2, 0 },
    { 26, 122, 506, 142, button_trade_resource, button_none, 0, 0, 0, 1, 3, 0 },
    { 26, 144, 506, 164, button_trade_resource, button_none, 0, 0, 0, 1, 4, 0 },
    { 26, 166, 506, 186, button_trade_resource, button_none, 0, 0, 0, 1, 5, 0 },
    { 26, 188, 506, 208, button_trade_resource, button_none, 0, 0, 0, 1, 6, 0 },
    { 26, 210, 506, 230, button_trade_resource, button_none, 0, 0, 0, 1, 7, 0 },
    { 26, 232, 506, 252, button_trade_resource, button_none, 0, 0, 0, 1, 8, 0 },
    { 26, 254, 506, 274, button_trade_resource, button_none, 0, 0, 0, 1, 9, 0 },
    { 26, 276, 506, 296, button_trade_resource, button_none, 0, 0, 0, 1, 10, 0 },
    { 26, 298, 506, 318, button_trade_resource, button_none, 0, 0, 0, 1, 11, 0 },
    { 26, 320, 506, 340, button_trade_resource, button_none, 0, 0, 0, 1, 12, 0 },
    { 26, 342, 506, 362, button_trade_resource, button_none, 0, 0, 0, 1, 13, 0 },
    { 26, 364, 506, 384, button_trade_resource, button_none, 0, 0, 0, 1, 14, 0 } };

resource_settings_button_tables resource_settings_buttons = {
    .industry = {
            .buttons = {
                { 98, 250, 530, 280, window_resource_settings_toggle_industry, button_none, 0, 0, 0, 1, 0, 0 }
            },
            .list_end = 0
        },
    .trade = {
            .buttons = {
                { 98, 212, 530, 242, window_resource_settings_cycle_trade, button_none, 0, 0, 0, 1, 0, 0 }
            },
            .list_end = 0
        },
    .import_toggle = {
            .buttons = {
                { 26, 212, 317, 242, window_resource_settings_toggle_import, button_none, 0, 0, 0, 1, 0, 0 }
            },
            .list_end = 0
        },
    .export_toggle = {
            .buttons = {
                { 321, 212, 612, 242, window_resource_settings_toggle_export, button_none, 0, 0, 0, 1, 0, 0 }
            },
            .list_end = 0
        },
    .stockpile = {
            .buttons = {
                { 98, 288, 530, 338, window_resource_settings_toggle_stockpile, button_none, 0, 0, 0, 1, 0, 0 }
            },
            .list_end = 0
        }
};

arrow_button resource_settings_export_arrows[2] = { { 561, 215, 18, 24, window_resource_settings_change_trade_amount, 0, 0, 0, 4, 1,
                                                                0 },
    { 585, 215, 16, 24, window_resource_settings_change_trade_amount, 0, 0, 0, 4, 0, 0 } };

arrow_button resource_settings_import_arrows[2] = { { 266, 215, 18, 24, window_resource_settings_change_trade_amount, 0, 0, 0, 4, 1,
                                                                0 },
    { 290, 215, 16, 24, window_resource_settings_change_trade_amount, 0, 0, 0, 4, 0, 0 } };

image_button resource_settings_image_buttons[2] = {
    { 26, 332, 27, 27, 4, 403, 0, 0, window_resource_settings_help, button_none, 1, 0, 0, 0, 0, 0 },
    { 590, 335, 24, 24, 4, 403, 4, 0, window_resource_settings_back, button_none, 1, 0, 0, 0, 0, 0 },
};

image_button buttons_expanded_extra[2] = { { 128, 0, 33, 20, 4, 379, 7, 0, sidebar_collapse, button_none, 1, 0, 0, 0, 0,
                                        0 },
    { 8, -5, 117, 25, 4, 362, 0, 0, window_overlay_menu_show, button_message_help, 1, 0, 0, 0, 0, 18 } };

single_image_button buttons_collapsed_expand = {
    .buttons = {
        { 9, 0, 33, 20, 4, 379, 10, 0, sidebar_collapse, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

image_button buttons_expanded_build[16] = { { 9, 251, 34, 48, 2, 405, 0, 0, sidebar_build_button_click,
    button_message_help, 1, 0, 0, 0, 1, 110 },
    { 9, 300, 34, 48, 2, 405, 4, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 3, 110 },
    { 9, 351, 34, 48, 2, 405, 8, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 2, 110 },
    { 46, 251, 34, 48, 2, 405, 12, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 4, 110 },
    { 46, 300, 34, 48, 2, 405, 16, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 10, 110 },
    { 46, 351, 34, 48, 2, 405, 20, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 12, 110 },
    { 86, 251, 34, 48, 2, 405, 24, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 8, 110 },
    { 86, 300, 34, 48, 2, 405, 28, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 6, 110 },
    { 86, 351, 34, 48, 2, 405, 32, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 7, 110 },
    { 125, 251, 34, 48, 2, 405, 36, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 5, 110 },
    { 125, 300, 34, 48, 2, 405, 40, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 9, 110 },
    { 125, 351, 34, 48, 2, 405, 44, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 11, 110 },
    { 9, 404, 35, 45, 4, 405, 48, 0, city_button_undo, button_none, 1, 0, 0, 0, 0, 0 },
    { 46, 404, 38, 45, 4, 405, 52, 0, window_message_list_show, button_message_help, 1, 0, 0, 0, 0, 34 },
    { 86, 404, 28, 45, 2, 405, 56, 0, city_button_go_to_problem, button_none, 1, 0, 0, 0, 0, 0 },
    { 116, 404, 43, 45, 4, 405, 60, 0, mission_show_briefing, button_none, 1, 0, 0, 0, 0, 0 } };

image_button buttons_collapsed_build[12] = { { 9, 21, 27, 35, 2, 377, 0, 0, sidebar_build_button_click,
    button_message_help, 1, 0, 0, 0, 1, 110 },
    { 9, 57, 27, 34, 2, 377, 4, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 3, 110 },
    { 9, 92, 27, 36, 2, 377, 8, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 2, 110 },
    { 9, 129, 27, 33, 2, 377, 12, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 4, 110 },
    { 9, 163, 27, 34, 2, 377, 16, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 10, 110 },
    { 9, 198, 27, 34, 2, 377, 20, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 12, 110 },
    { 9, 233, 27, 32, 2, 377, 24, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 8, 110 },
    { 9, 266, 27, 35, 2, 377, 28, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 6, 110 },
    { 9, 302, 27, 34, 2, 377, 32, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 7, 110 },
    { 9, 338, 27, 35, 2, 377, 36, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 5, 110 },
    { 9, 374, 27, 31, 2, 377, 40, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 9, 110 },
    { 9, 406, 27, 39, 2, 377, 44, 0, sidebar_build_button_click, button_message_help, 1, 0, 0, 0, 11, 110 } };

sidebar_editor_button_table buttons_editor = {
    .buttons = {
        { 7, 123, 71, 23, 4, 406, 45, 0, editor_button_empire, button_none, 1, 0, 0, 0, 0, 0 },
        { 84, 123, 71, 23, 4, 406, 45, 0, editor_sidebar_preview, button_none, 1, 0, 0, 0, 1, 0 },
        { 13, 267, 39, 26, 2, 406, 0, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 0, 0 },
        { 63, 267, 39, 26, 2, 406, 3, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 1, 0 },
        { 113, 267, 39, 26, 2, 406, 6, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 2, 0 },
        { 13, 303, 39, 26, 2, 406, 18, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 6, 0 },
        { 63, 303, 39, 26, 2, 406, 30, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 10, 0 },
        { 113, 303, 39, 26, 2, 406, 15, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 5, 0 },
        { 13, 339, 39, 26, 2, 406, 33, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 17, 0 },
        { 63, 339, 39, 26, 2, 406, 39, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 13, 0 },
        { 113, 339, 39, 26, 2, 406, 42, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 14, 0 },
        { 13, 375, 39, 26, 2, 406, 27, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 24, 0 },
        { 63, 375, 39, 26, 4, 406, 24, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 8, 0 },
        { 113, 375, 39, 26, 2, 712, 0, 0, editor_sidebar_tool, button_none, 1, 0, 0, 0, 32, 0 },
        { 0 }
    },
    .list_end = 0
};

image_button buttons_expanded_top[2] = { { 16, 143, 60, 36, 4, 387, 0, 0, sidebar_button_advisors,
    button_message_help, 1, 0, 0, 0, 0, 14 },
    { 90, 143, 62, 36, 4, 387, 4, 0, editor_button_empire, button_message_help, 1, 0, 0, 0, 0, 32 } };

image_button rotate_button_x[1] = { { 0, 0, 36, 21, 4, 379, 0, 0, rotation_button_click, button_none, 1, 0, 0, 0, 0, 0 } };

image_button file_dialog_buttons[4] = { { 344, 335, 34, 34, 4, 365, 4, 0, file_dialog_button, button_none, 1, 0, 0, 0, 0, 0 },
    { 392, 335, 34, 34, 4, 365, 0, 0, file_dialog_button, button_none, 1, 0, 0, 0, 1, 0 },
    { 464, 120, 34, 34, 4, 365, 8, 0, file_dialog_button_scroll, button_none, 1, 0, 0, 0, 0, 0 },
    { 464, 300, 34, 34, 4, 365, 12, 0, file_dialog_button_scroll, button_none, 1, 0, 0, 0, 1, 0 } };

image_button map_dialog_buttons[4] = { { 344, 335, 34, 34, 4, 365, 0, 0, editor_map_dialog_button,
    button_none, 1, 0, 0, 0, 1, 0 },
    { 392, 335, 34, 34, 4, 365, 4, 0, editor_map_dialog_button, button_none, 1, 0, 0, 0, 0, 0 },
    { 464, 120, 34, 34, 4, 365, 8, 0, file_dialog_button_scroll, button_none, 1, 0, 0, 0, 0, 0 },
    { 464, 300, 34, 34, 4, 365, 12, 0, file_dialog_button_scroll, button_none, 1, 0, 0, 0, 1, 0 } };

image_button_triple building_info_help_close_buttons = {
    .buttons = {
        { 14, 0, 27, 27, 4, 403, 0, 0, window_building_info_help, button_none, 1, 0, 0, 0, 0, 0 },
        { 424, 3, 24, 24, 4, 403, 4, 0, building_info_legion_exit, button_none, 1, 0, 0, 0, 0, 0 },
        { 350, -38, 28, 28, 4, 375, 9, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 5, 0 }
    },
    .list_end = 0
};

image_button advisor_help_buttons[2] = { { 11, -7, 27, 27, 4, 403, 0, 0, advisor_button_help,
    button_none, 1, 0, 0, 0, 0, 0 },
    { 605, -3, 24, 24, 4, 403, 4, 0, button_advisor_bar, button_none, 1, 0, 0, 0, 0, 0 } };

image_button festival_panel_buttons[4] = { { 58, 316, 27, 27, 4, 403, 0, 0, button_festival_help,
    button_none, 1, 0, 0, 0, 0, 0 },
    { 558, 319, 24, 24, 4, 403, 4, 0, button_festival_close, button_none, 1, 0, 0, 0, 0, 0 },
    { 400, 317, 34, 34, 4, 365, 0, 0, button_festival_confirm, button_none, 1, 0, 0, 0, 1, 0 },
    { 358, 317, 34, 34, 4, 365, 4, 0, button_festival_close, button_none, 1, 0, 0, 0, 0, 0 } };

image_button hold_festival_buttons[4] = { { 58, 316, 27, 27, 4, 403, 0, 0, button_festival_help,
    button_none, 1, 0, 0, 0, 0, 0 },
    { 558, 319, 24, 24, 4, 403, 4, 0, button_festival_close, button_none, 1, 0, 0, 0, 0, 1 },
    { 400, 317, 34, 34, 4, 365, 0, 0, button_festival_confirm, button_none, 1, 0, 0, 0, 1, 1 },
    { 358, 317, 34, 34, 4, 365, 4, 0, button_festival_close, button_none, 1, 0, 0, 0, 0, 1 } };

single_image_button empire_help_button = {
    .buttons = {
        { 0, 0, 27, 27, 4, 403, 0, 0, button_empire_help, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button empire_exit_button = {
    .buttons = {
        { 0, 0, 24, 24, 4, 403, 4, 0, button_empire_exit, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button empire_advisors_button = {
    .buttons = {
        { -4, 0, 28, 28, 4, 375, 12, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 6, 0 }
    },
    .list_end = 0
};

image_button city_intro_buttons[2] = { { 92, 248, 28, 28, 4, 375, 12, 0, window_advisors_show_checked,
    button_none, 1, 0, 0, 0, 6, 0 },
    { 522, 252, 24, 24, 4, 403, 4, 0, window_go_empire, button_none, 1, 0, 0, 0, 0, 0 } };

single_image_button message_back = {
    .buttons = {
        { 0, 0, 31, 20, 4, 359, 8, 0, message_dialog_back, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button message_close = {
    .buttons = {
        { 0, 0, 24, 24, 4, 403, 4, 0, window_message_dialog_close, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

message_scroll_button message_scroll_up = {
    .button = { .buttons = { { 0, 0, 39, 26, 6, 365, 8, 0, message_dialog_scroll_arrow, button_none, 1, 0, 0, 0, 0, 0 } } },
    .list_end = 0
};

message_scroll_button message_scroll_down = {
    .button = { .buttons = { { 0, 0, 39, 26, 6, 365, 12, 0, message_dialog_scroll_arrow, button_none, 1, 0, 0, 0, 1, 0 } } },
    .list_end = 0
};

single_image_button message_help = {
    .buttons = {
        { 0, 0, 18, 27, 4, 403, 0, 0, message_dialog_show_help, button_none, 1, 0, 0, 0, 1, 0 }
    },
    .list_end = 0
};

single_image_button message_go_to_problem = {
    .buttons = {
        { 0, 0, 27, 27, 4, 462, 0, 0, message_dialog_go_to, button_none, 1, 0, 0, 0, 1, 0 }
    },
    .list_end = 0
};

single_image_button message_list_help = {
    .buttons = {
        { 0, 0, 27, 27, 4, 403, 0, 0, message_list_button_help, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button message_labor_advisor = {
    .buttons = {
        { 0, 0, 27, 27, 4, 375, 0, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 1, 0 }
    },
    .list_end = 0
};

single_image_button message_military_advisor[2] = {
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 3, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 2, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 3, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 3, 0 }
        },
        .list_end = 0
    }
};

single_image_button message_imperial_advisor[2] = {
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 6, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 4, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 9, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 5, 0 }
        },
        .list_end = 0
    }
};

single_image_button message_trade_advisor = {
    .buttons = {
        { 0, 0, 27, 27, 4, 375, 12, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 6, 0 }
    },
    .list_end = 0
};

single_image_button message_population_advisor = {
    .buttons = {
        { 0, 0, 27, 27, 4, 375, 15, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 7, 0 }
    },
    .list_end = 0
};

single_image_button message_health_advisor[3] = {
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 18, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 8, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 21, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 9, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 24, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 10, 0 }
        },
        .list_end = 0
    }
};

single_image_button message_religion_advisor[4] = {
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 27, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 11, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 30, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 12, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 33, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 13, 0 }
        },
        .list_end = 0
    },
    {
        .buttons = {
            { 0, 0, 27, 27, 4, 375, 36, 0, window_advisors_show_checked, button_none, 1, 0, 0, 0, 14, 0 }
        },
        .list_end = 0
    }
};

single_image_button message_list_close = {
    .buttons = {
        { 0, 0, 24, 24, 4, 403, 4, 0, window_go_city, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button message_list_scroll_up = {
    .buttons = {
        { 0, 0, 39, 26, 6, 365, 8, 0, message_list_scroll, button_none, 1, 0, 0, 0, 0, 0 }
    },
    .list_end = 0
};

single_image_button message_list_scroll_down = {
    .buttons = {
        { 0, 0, 39, 26, 6, 365, 12, 0, message_list_scroll, button_none, 1, 0, 0, 0, 1, 0 }
    },
    .list_end = 0
};

image_button popup_buttons[2] = { { 256, 100, 34, 34, 4, 365, 0, 0, popup_dialog_button_ok, button_none, 1, 0, 0, 0, 1, 0 },
    { 192, 100, 34, 34, 4, 365, 4, 0, popup_dialog_button_cancel, button_none, 1, 0, 0, 0, 0, 0 } };

image_button difficulty_arrow_buttons[2] = { { 0, 0, 17, 17, 4, 481, 0, 0, difficulty_options_arrow,
    button_none, 1, 0, 0, 0, 0, 0 },
    { 17, 0, 17, 17, 4, 285, 0, 0, difficulty_options_arrow, button_none, 1, 0, 0, 0, 1, 0 } };

storage_order_button_table order_buttons = {
    .limit_buttons = {
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 0 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 1 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 1 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 2 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 2 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 3 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 3 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 4 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 4 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 5 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 5 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 6 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 6 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 7 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 7 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 8 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 8 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 9 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 9 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 10 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 10 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 11 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 11 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 12 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 12 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 13 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 13 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 14 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 14 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 15 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 15 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 16 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 16 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 17 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 17 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 18 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 18 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 19 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 19 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 20 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 20 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 21 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 21 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 22 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 22 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 23 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 23 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 24 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 24 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 25 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 25 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 26 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 26 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 27 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 27 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 28 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 28 },
        { 0, 0, 17, 17, 4, 285, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 1, 29 },
        { 17, 0, 17, 17, 4, 481, 0, 0, storage_orders_limit_arrow, button_none, 1, 0, 0, 0, 0, 29 }
    },
    .blank_buttons = {
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 },
        { 0 }
    },
    .editor_terrain_arrows = {
        { 160, 452, 19, 24, editor_map_terrain_reload, 0, 0, 0, 4, 1, 0 },
        { 256, 452, 21, 24, editor_map_terrain_reload, 0, 0, 0, 4, 0, 0 },
        { 320, 452, 25, 24, button_none, 0, 0, 0, 5, 0, 0 }
    }
};

mission_selection_table_group mission_selection_tables = {
    .mission_selection_buttons = { { 600, 440, 27, 27, 4, 461, 0, 0, mission_selection_start, button_none, 1,
                                               0, 0, 0, 1, 0 },
        { 335, 440, 27, 27, 4, 365, 4, 0, map_selection_back, button_none, 1, 0, 0, 0, 1, 0 } },
    .mission_buttons = { { 20, 291, 164, 316, mission_selection_select, button_none, 0, 0, 0, 1, 0, 0 },
        { 20, 321, 164, 346, mission_selection_select, button_none, 0, 0, 0, 1, 1, 0 },
        { 172, 261, 316, 286, mission_selection_select, button_none, 0, 0, 0, 1, 2, 0 },
        { 172, 291, 316, 316, mission_selection_select, button_none, 0, 0, 0, 1, 3, 0 },
        { 172, 321, 316, 346, mission_selection_select, button_none, 0, 0, 0, 1, 4, 0 },
        { 20, 391, 164, 416, mission_selection_select, button_none, 0, 0, 0, 1, 5, 0 },
        { 20, 421, 164, 446, mission_selection_select, button_none, 0, 0, 0, 1, 6, 0 },
        { 172, 391, 316, 416, mission_selection_select, button_none, 0, 0, 0, 1, 7, 0 },
        { 172, 421, 316, 446, mission_selection_select, button_none, 0, 0, 0, 1, 8, 0 },
        { 26, 208, 206, 233, mission_selection_proceed, button_none, 0, 0, 0, 1, 0, 0 },
        { 210, 208, 296, 233, window_mission_selection_show_all, button_none, 0, 0, 0, 1, 0, 0 } }
};

player_selection_table_group player_selection_tables = {
    .player_selection_image_buttons = { { 464, 120, 34, 34, 4, 365, 8, 0, player_selection_button_scroll, button_none,
                                            1, 0, 0, 0, 0, 0 },
        { 464, 300, 34, 34, 4, 365, 12, 0, player_selection_button_scroll, button_none, 1, 0, 0, 0, 1, 0 } },
    .player_row_buttons = { { 160, 128, 448, 144, player_selection_button_select, button_none, 0, 0, 0, 1, 0, 0 },
        { 160, 144, 448, 160, player_selection_button_select, button_none, 0, 0, 0, 1, 1, 0 },
        { 160, 160, 448, 176, player_selection_button_select, button_none, 0, 0, 0, 1, 2, 0 },
        { 160, 176, 448, 192, player_selection_button_select, button_none, 0, 0, 0, 1, 3, 0 },
        { 160, 192, 448, 208, player_selection_button_select, button_none, 0, 0, 0, 1, 4, 0 },
        { 160, 208, 448, 224, player_selection_button_select, button_none, 0, 0, 0, 1, 5, 0 },
        { 160, 224, 448, 240, player_selection_button_select, button_none, 0, 0, 0, 1, 6, 0 },
        { 160, 240, 448, 256, player_selection_button_select, button_none, 0, 0, 0, 1, 7, 0 },
        { 160, 256, 448, 272, player_selection_button_select, button_none, 0, 0, 0, 1, 8, 0 },
        { 160, 272, 448, 288, player_selection_button_select, button_none, 0, 0, 0, 1, 9, 0 },
        { 160, 288, 448, 304, player_selection_button_select, button_none, 0, 0, 0, 1, 10, 0 },
        { 160, 304, 448, 320, player_selection_button_select, button_none, 0, 0, 0, 1, 11, 0 },
        { 142, 332, 267, 357, player_selection_button_new_player, button_none, 0, 0, 0, 1, 0, 0 },
        { 276, 332, 406, 357, player_selection_button_delete, button_none, 0, 0, 0, 1, 0, 0 },
        { 418, 332, 498, 357, player_selection_button_proceed, button_none, 0, 0, 0, 1, 0, 0 },
        { 196, 362, 446, 387, editor_menu_exit_builder, button_none, 0, 0, 0, 1, 0, 0 } }
};


int32_t overlay_menu_hover_time;

int32_t saved_scroll_speed;

file_list file_list_saves;

int32_t scenario_current_request;

int32_t display_mode_confirm_time;

int32_t advisor_labor_priority_max;

int32_t labor_focus;

int32_t trade_resource_index;

int32_t empire_goods_list_slot;

int32_t construction_place_ok;

int32_t needs_groundwater;

int32_t needs_road;

int32_t needs_wall;

int32_t needs_marsh;

int32_t needs_water_near;

int32_t needs_trees;

int32_t needs_farm_land;

int32_t construction_start_grid_offset;

int32_t construction_end_y;

int32_t construction_start_y;

int32_t construction_end_x;

int32_t construction_start_x;

int32_t construction_in_progress;

int32_t construction_building_type;

int32_t file_dialog_scroll_layout;

int32_t file_dialog_selected;

int32_t file_dialog_scroll;

int32_t file_dialog_scroll_total;

int32_t name_list_choice;

int32_t debug_event_kind;

int32_t debug_event_level;

int32_t debug_event_text;

int32_t cheat_allow_all;

int32_t editor_empire_file_number;

int32_t editor_empire_map_number;

int32_t editor_new_map_size;

int32_t editor_herd_point_number;

int32_t editor_landing_point_number;

int32_t editor_fishing_point_number;

int32_t editor_invasion_point_number;

int32_t editor_tool_variant;

int32_t editor_tool_kind;

int32_t editor_brush_size;

int32_t editor_map_preview;

int32_t editor_active;

int32_t city_population_graph_order;

int32_t advisor_shown;

terrain_test construction_near_terrain_test;

input_dialog_callback numeric_input_on_ok;

input_dialog_callback select_list_on_choose;

int32_t select_list_extra_text;

int32_t select_list_extra_on;

uint8_t construction_hover_locked;

uint32_t game_options_flags;

uint8_t overlay_show_all;

video_player *video_player_ptr;

int32_t build_menu_item_counts[12];

int32_t mission_select_mode;

int32_t ui_panel_y;

int32_t ui_panel_x;

int32_t intermezzo_next_window;

int32_t intermezzo_end_ms;

static_assert(sizeof(mission_records_state) == 80, "record_savings is the player file's mission records of 80 bytes");
mission_records_state record_savings;

static_assert(sizeof(player_data) == 19100, "campaign_object is the player file's record of 19100 bytes");
player_data campaign_object;

campaign_mission_choice_slot campaign_mission_choice;

int32_t mission_briefing_started;

int32_t mission_selected;

int32_t campaign_period_reached;

uint8_t game_is_replay;

const char empty_text[4] = "";

int32_t cheat_focus;

map_list map_file_list;

int32_t arrow_button_focus_id;

int32_t image_button_focus_id;

int32_t focus_button_id_tertiary;

int32_t focus_button_id_secondary;

int32_t focus_button_id;

int32_t overlay_submenu_index;

int32_t overlay_submenu_count;

int32_t overlay_submenu_open;


int32_t build_menu_y_offset;

int32_t build_menu_group;

int32_t build_menu_item_count;

popup_state popup;

int32_t menu_enabled[1140];

int32_t empire_scroll_y;

int32_t empire_scroll_x;

int32_t empire_ui_mode;

static_assert(sizeof(scenario_record) == 1592, "scenario_data is the savegame and scenario record of 1592 bytes");
scenario_record scenario_data;

int32_t empire_selected_object;

int32_t empire_selected_city;

static_assert(sizeof(scenario_events_t) == 18636, "scenario_events is the savegame and scenario events of 18636 bytes");
scenario_events_t scenario_events;

int32_t formation_selected;

static_assert(sizeof(formation_table) == 7200, "formation_array is the savegame's formation table of 7200 bytes");
formation_table formation_array;

int32_t selected_transport_id;

int32_t selected_warship_id;

int32_t selected_transport_active;

int32_t selected_warship_active;

char high_score_previous_name[32];

int32_t hs_result;

static_assert(sizeof(mission_record_table) == 7600, "highscores is the high score file's table of 7600 bytes");
mission_record_table highscores;

int32_t hs_scroll;

int32_t hs_count;

int32_t tooltip_window_id;

int32_t dialog_history_count;

int32_t dialog_history_scroll[200];

int32_t dialog_history_id[200];

int32_t dialog_text_id;

int32_t dialog_scroll_drag_y;

int32_t dialog_scroll_dragging;

int32_t rich_scroll_pos;

int32_t dialog_scroll_max;

int32_t rich_line_count;

int32_t rich_viewport_lines;

int32_t dialog_background;

int32_t dialog_text_height_blocks;

int32_t dialog_text_width_blocks;

int32_t dialog_y_text;

int32_t dialog_x_text;

int32_t dialog_x;

int32_t rich_text_link_font;

int32_t rich_text_normal_font;

int32_t dialog_param2;

int32_t dialog_param1;

int32_t dialog_month;

int32_t dialog_year;

int32_t dialog_prev_window;

int32_t dialog_show_video;

uint8_t menu_can_start;

uint8_t menu_period_open;

uint8_t menu_has_saves;

uint8_t menu_can_continue;

int32_t button_parameter2;

int32_t button_parameter;


uint8_t file_dialog_error_frames;

window_return_slot file_dialog_return_window;

uint8_t file_dialog_type;

uint8_t popup_answer;

uint8_t game_quit_request;

int32_t image_view_y;

int32_t image_view_x;

int32_t image_view_height;

int32_t image_view_width;


int32_t g_word_chars;

int32_t g_text_width;

uint8_t capture_insert_mode;

int32_t keyboard_field_index;

int32_t keyboard_accepted;

char scenario_map_name[65];

char file_dialog_last_name[68];

char file_dialog_name[6568];

char file_extension[660];

int32_t display_width;


int32_t ui_origin_y;

int32_t ui_origin_x;

uint8_t window_ui_redraw;

uint8_t window_redraw_request;


uint8_t window_current_id;

int32_t window_input_delay_frames;

static_assert(sizeof(settings_file) == 564, "settings_data is the settings file's record of 564 bytes");
settings_file settings_data;

cursor_handle_table cursor_handles;

void *cursor_current;

int32_t app_paused;

int32_t app_active;

void *platform_main_window;

int32_t display_mode;


int32_t key_scroll_right;

int32_t key_scroll_left;

int32_t key_scroll_down;

int32_t key_scroll_up;

uint32_t city_scroll_last_time;


int32_t inner_width;

int32_t inner_y;

int32_t inner_x;

int32_t thumb_y;

int32_t thumb_dragged;


uint8_t mission_lost_can_replay;

uint8_t city_lost_by_population;

int32_t month_axis_end_year;

int32_t month_axis_end_month;

int32_t month_axis_start_year;

int32_t month_axis_start_month;

int32_t city_houses_of_win_level;

int32_t bi_progress_tomb[8];

mausoleum_info_progress bi_progress_mausoleum;

scripted_monument_info_progress bi_progress_lighthouse;

scripted_monument_info_progress bi_progress_caesareum;

abu_simbel_info_progress bi_progress_abu_simbel;

int32_t bi_progress_sun_temple[14];

pyramid_progress bi_progress_pyramid;

int32_t bi_storage_state;

int32_t building_info_orders_shown;

int32_t building_info_warship_id;

int32_t building_info_transport_id;

int32_t building_info_formation_id;

int32_t info_has_academy;

int32_t house_worst_desirability_building;

int32_t info_figure_index;

int32_t building_info_advisor_buttons[15];

int32_t info_figure_ids[7];

int32_t info_figure_count;

int32_t building_info_details_shown;

int32_t info_terrain_kind;

int32_t building_info_mode;

int32_t info_help_id;

building_info_details bi_road_access_block;

int32_t building_info_building_id;

int32_t building_info_h;

int32_t info_w_blocks;

int32_t building_info_y;

int32_t building_info_x;

int32_t info_first_draw;

file_list player_file_list;


int32_t plist_scroll;

int32_t plist_selected;

uint8_t message_box_answered;

int32_t message_box_button;

int32_t message_box_button_count;

empire_event_marker_state empire_markers;


int32_t victory_y;

int32_t victory_x;

city_resource_list city_foods;

city_resource_list city_resources;

int32_t advisor_height_blocks;


int32_t advisor_y;

int32_t advisor_x;

int32_t label_animation[16];

image_collection_object image_collection;
