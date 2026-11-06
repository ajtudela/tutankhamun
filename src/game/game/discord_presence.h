// Discord Rich Presence of a normal play (discord_presence.c) and its settings of discord.json: the application id
// and whether the presence is on.

#ifndef GAME_DISCORD_PRESENCE_H
#define GAME_DISCORD_PRESENCE_H

#include "platform/discord_rpc.h"

#include <stdint.h>

enum discord_presence_record {
    DISCORD_PRESENCE_DISCORD = 0,
    DISCORD_PRESENCE_RECORDS = 1,
};

typedef struct discord_presence_settings {
    char app_id[DISCORD_RPC_APP_ID_SIZE];
    int32_t enabled;
} discord_presence_settings;

extern discord_presence_settings discord_presence_config[DISCORD_PRESENCE_RECORDS];

void discord_presence_start(void);
void discord_presence_update(void);
void discord_presence_stop(void);

#endif
