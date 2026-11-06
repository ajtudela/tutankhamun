// Discord Rich Presence over the Discord client's local pipe (discord_rpc.c): start the worker with the application
// id, hand it the activity's two lines in the game's code page, stop it at exit.
// A C port of Akhenaten's src/platform/discord_rpc.h (https://github.com/dalerank/Akhenaten, commit 8ec540693).
// Copyright (C) the Akhenaten authors. Licence: GNU Affero General Public License v3.0 (LICENSES/AGPL-3.0.txt).

#ifndef PLATFORM_DISCORD_RPC_H
#define PLATFORM_DISCORD_RPC_H

#include <stdbool.h>

enum discord_rpc_limits {
    DISCORD_RPC_APP_ID_SIZE = 32,
    DISCORD_RPC_TEXT_SIZE = 128,
};

void discord_rpc_set_verbose(bool verbose);
bool discord_rpc_start(const char *app_id);
bool discord_rpc_running(void);
void discord_rpc_set_activity(const char *details, const char *state);
void discord_rpc_stop(void);

#endif
