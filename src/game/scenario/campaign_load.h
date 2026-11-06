// The functions of scenario/campaign_load.c: the campaign file's parse helpers and the campaign object's methods.

#ifndef SCENARIO_CAMPAIGN_LOAD_H
#define SCENARIO_CAMPAIGN_LOAD_H

#include "scenario/campaign.h"

#include <stdbool.h>
#include <stdint.h>

void campaign_strip_whitespace(char *text);
int32_t campaign_parse_int(int32_t *out);
int32_t campaign_parse_short(int16_t *out);
void campaign_log_error(const char *path, int32_t line, const char *text, const char *error);
void campaign_link_init(campaign_link *link);
bool campaign_add_link(campaign *self, const campaign_link *link);
int32_t campaign_find_link(campaign *self, int32_t period, int32_t index, int32_t choice);
bool campaign_load(campaign *self, const char *path);

#endif
