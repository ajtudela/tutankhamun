// The initial data of the platform: window, display and file strings, as C tables. platform/platform_data.h declares
// them.

#include "platform/platform_data.h"

const char campaign_file_name[684] = "campaign.txt\0\0\0\0"
                             "ERR:Exited Pharaoh, could not load tax sentiment data.\0\0"
                             "ERR:Exited Pharaoh, could not load figures.\0"
                             "ERR:Exited Pharaoh, could not load model.\0\0\0"
                             "OK :Pharaoh setup.\0\0"
                             "ERR:Exited Pharaoh, bmp files have changed since sg2 was last run. Need to rerun sg2 abd get new project size\0\0\0"
                             "ERR:Exited Pharaoh, Could not find 'the_empire.dat' file.\0\0\0"
                             "ERR:Exited Pharaoh, Could not find main 'dat' file.\0"
                             "ERR:Exited Pharaoh, insufficient memory to run.\0"
                             "OK :Direct draw initialised.\0\0\0\0"
                             "ERR:Exited Pharaoh, couldn't start direct draw.\0"
                             "OK :Game text loaded.\0\0\0"
                             "ERR: Could not detect Pharaoh CD!!!\0"
                             "ERR:Exited Pharaoh, 'Pharaoh_Text.eng' or 'Pharaoh_MM.eng' files not found or too large.";

const char text_file_name[20] = "Pharaoh_Text.eng";

const char message_file_name[76] = "Pharaoh_MM.eng\0\0"
                            "ERR:Exited Pharaoh, could not find CD drive information.";

int32_t desktop_height;

int32_t desktop_width;
