// The program's own update (updater.c), done inside the program, as Discord or Claude do it: at start a thread reads
// the version of the newest build in the GitHub release "nightly" (written there by the CI), downloads the newer
// build beside the program (update\) and checks it; the front menu shows it ready, and when the game closes the
// program puts it in its own place, so the next start is the new build. The running program cannot be overwritten on
// Windows but it can be renamed: it becomes tutankhamun.exe.old, removed at the next start. "Restart now" closes the
// game and starts the new program at once.
//
// Only the CI builds of Windows update themselves (TUTANKHAMUN_UPDATER, CMakeLists.txt); a build of one's own, the
// other systems and Android (Google Play updates it) show the version alone. tutankhamun.ini [update] check=0 or
// --no-update turns the check off. TUTANKHAMUN_UPDATE_URL (a folder URL ending in "/") replaces the release for a test.

#ifndef PLATFORM_UPDATER_H
#define PLATFORM_UPDATER_H

#include <stdint.h>

typedef enum updater_state {
    UPDATER_OFF,
    UPDATER_CHECKING,
    UPDATER_NO_ANSWER,
    UPDATER_UP_TO_DATE,
    UPDATER_DOWNLOADING,
    UPDATER_READY,
    UPDATER_FAILED,
} updater_state;

void updater_disable(void);
// at the start of the play mode, before the game directory is entered
void updater_start(void);
updater_state updater_current_state(void);
int32_t updater_progress_percent(void);
int32_t updater_new_build(void);
// the front menu's "restart now": the game is to close, then the new program starts
void updater_restart_request(void);
// after the game ends: a ready update takes the program's place, and the new program starts when asked
void updater_finish(void);

#endif
