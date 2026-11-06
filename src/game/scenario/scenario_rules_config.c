// The gameplay config tables of the scenario events (core/config.h): request messages, earthquake steps, troops
// request losses, reason phrase variant texts, event types, event type texts and invasion warnings.

#include "core/config.h"
#include "scenario/event_helpers.h"
#include "scenario/scenario_rules.h"

enum scenario_rules_strides {
    REQUEST_MESSAGE_STRIDE = sizeof(request_message_rule) / sizeof(int32_t),
    EARTHQUAKE_STEP_STRIDE = sizeof(earthquake_step) / sizeof(int32_t),
    TROOPS_LOSS_STRIDE = sizeof(troops_loss_bracket) / sizeof(int32_t),
    EVENT_TYPE_STRIDE = sizeof(event_type_property) / sizeof(int32_t),
    EVENT_TYPE_TEXT_STRIDE = sizeof(event_type_text) / sizeof(int32_t),
    INVASION_WARNING_STRIDE = sizeof(invasion_warning) / sizeof(int32_t),
    MESSAGE_TYPE_MAX = 0x7FFF,
    TEXT_MAX = 0x7FFF,
    TEXT_GROUP_MAX = 0x7FFF,
};

static const config_field request_message_fields[] = {
    { "announcement", request_message_rules[0].announcement, REQUEST_MESSAGE_STRIDE, EVENT_SENDERS, -1, TEXT_MAX, 0 },
    { "completed", request_message_rules[0].completed, REQUEST_MESSAGE_STRIDE, EVENT_SENDERS, -1, TEXT_MAX, 0 },
    { "refused", request_message_rules[0].refused, REQUEST_MESSAGE_STRIDE, EVENT_SENDERS, -1, TEXT_MAX, 0 },
    { "too_late", request_message_rules[0].too_late, REQUEST_MESSAGE_STRIDE, EVENT_SENDERS, -1, TEXT_MAX, 0 },
    { "defeat", request_message_rules[0].defeat, REQUEST_MESSAGE_STRIDE, EVENT_SENDERS, -1, TEXT_MAX, 0 },
    { "completed_group", &request_message_rules[0].completed_group, REQUEST_MESSAGE_STRIDE, 1, -1, 53, 0 },
    { "refused_group", &request_message_rules[0].refused_group, REQUEST_MESSAGE_STRIDE, 1, -1, 53, 0 },
    { "too_late_group", &request_message_rules[0].too_late_group, REQUEST_MESSAGE_STRIDE, 1, -1, 53, 0 },
    { "defeat_group", &request_message_rules[0].defeat_group, REQUEST_MESSAGE_STRIDE, 1, -1, 53, 0 },
    { "reason_column", &request_message_rules[0].reason_column, REQUEST_MESSAGE_STRIDE, 1, -1, 35, 0 },
};

static const config_storage request_message_storage[] = {
    { request_message_rules, sizeof(request_message_rules) },
};

const config_table request_message_table = {
    "request_messages.json",
    "subtype",
    0,
    REQUEST_SUBTYPES,
    CONFIG_TABLE_COLUMNS(request_message_fields, request_message_storage),
};

static const config_field earthquake_step_fields[] = {
    { "front", &earthquake_steps[0].front, EARTHQUAKE_STEP_STRIDE, 1, 0, 3, 0 },
    { "dx", &earthquake_steps[0].dx, EARTHQUAKE_STEP_STRIDE, 1, -1, 1, 0 },
    { "dy", &earthquake_steps[0].dy, EARTHQUAKE_STEP_STRIDE, 1, -1, 1, 0 },
};

static const config_storage earthquake_step_storage[] = {
    { earthquake_steps, sizeof(earthquake_steps) },
};

const config_table earthquake_step_table = {
    "earthquake_steps.json",
    "step",
    0,
    EARTHQUAKE_STEPS,
    CONFIG_TABLE_COLUMNS(earthquake_step_fields, earthquake_step_storage),
};

static const config_field troops_loss_fields[] = {
    { "below_percent", &troops_loss_brackets[0].below_percent, TROOPS_LOSS_STRIDE, 1, 0, 0x7FFFFFFF, 0 },
    { "losses", &troops_loss_brackets[0].losses, TROOPS_LOSS_STRIDE, 1, 0, 100, 0 },
};

static const config_storage troops_loss_storage[] = {
    { troops_loss_brackets, sizeof(troops_loss_brackets) },
};

const config_table troops_loss_table = {
    "troops_request_losses.json",
    "bracket",
    0,
    TROOPS_LOSS_BRACKETS,
    CONFIG_TABLE_COLUMNS(troops_loss_fields, troops_loss_storage),
};

static const config_field reason_phrase_variant_fields[] = {
    { "text", reason_phrase_variant_texts, 1, 1, 0, TEXT_MAX, 0 },
};

static const config_storage reason_phrase_variant_storage[] = {
    { reason_phrase_variant_texts, sizeof(reason_phrase_variant_texts) },
};

const config_table reason_phrase_variant_table = {
    "reason_phrase_variant_texts.json",
    "phrase_case",
    0,
    REASON_PHRASE_VARIANT_TEXTS,
    CONFIG_TABLE_COLUMNS(reason_phrase_variant_fields, reason_phrase_variant_storage),
};

static const config_field event_type_fields[] = {
    { "uses_city", &event_type_properties[0].uses_city, EVENT_TYPE_STRIDE, 1, 0, 1, 0 },
    { "is_invasion", &event_type_properties[0].is_invasion, EVENT_TYPE_STRIDE, 1, 0, 1, 0 },
    { "shows_items", &event_type_properties[0].shows_items, EVENT_TYPE_STRIDE, 1, 0, 1, 0 },
    { "uses_resource", &event_type_properties[0].uses_resource, EVENT_TYPE_STRIDE, 1, 0, 1, 0 },
    { "item_min", &event_type_properties[0].item_min, EVENT_TYPE_STRIDE, 1, 0, 0xFF, 0 },
    { "item_max", &event_type_properties[0].item_max, EVENT_TYPE_STRIDE, 1, 0, 0xFF, 0 },
    { "item_text_group", &event_type_properties[0].item_text_group, EVENT_TYPE_STRIDE, 1, -1, TEXT_GROUP_MAX, 0 },
    { "has_amount", &event_type_properties[0].has_amount, EVENT_TYPE_STRIDE, 1, 0, 1, 0 },
};

static const config_storage event_type_storage[] = {
    { event_type_properties, sizeof(event_type_properties) },
};

const config_table event_type_table = {
    "event_types.json",
    "type",
    config_event_type_names,
    EVENT_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(event_type_fields, event_type_storage),
};

static const config_field event_type_text_fields[] = {
    { "announcement", &event_type_texts[0].announcement, EVENT_TYPE_TEXT_STRIDE, 1, -1, TEXT_MAX, 0 },
    { "completed", &event_type_texts[0].completed, EVENT_TYPE_TEXT_STRIDE, 1, -1, TEXT_MAX, 0 },
    { "reason_group", &event_type_texts[0].reason_group, EVENT_TYPE_TEXT_STRIDE, 1, -1, SCENARIO_REASON_GROUPS - 1, 0 },
    { "reason_column", &event_type_texts[0].reason_column, EVENT_TYPE_TEXT_STRIDE, 1, -1, SCENARIO_REASON_COLUMNS - 1, 0 },
    { "first_phrase", &event_type_texts[0].first_phrase, EVENT_TYPE_TEXT_STRIDE, 1, -1, TEXT_MAX, 0 },
    { "second_phrase", &event_type_texts[0].second_phrase, EVENT_TYPE_TEXT_STRIDE, 1, -1, TEXT_MAX, 0 },
};

static const config_storage event_type_text_storage[] = {
    { event_type_texts, sizeof(event_type_texts) },
};

const config_table event_type_text_table = {
    "event_type_texts.json",
    "type",
    config_event_type_names,
    EVENT_TYPE_COUNT,
    CONFIG_TABLE_COLUMNS(event_type_text_fields, event_type_text_storage),
};

static const config_field invasion_warning_fields[] = {
    { "months_left", &invasion_warnings[0].months_left, INVASION_WARNING_STRIDE, 1, 0, 0x7FFF, 0 },
    { "phrase", invasion_warnings[0].phrase, INVASION_WARNING_STRIDE, EVENT_INVADERS, -1, TEXT_MAX, 0 },
    { "message", invasion_warnings[0].message, INVASION_WARNING_STRIDE, EVENT_INVADERS, 0, MESSAGE_TYPE_MAX, 0 },
};

static const config_storage invasion_warning_storage[] = {
    { invasion_warnings, sizeof(invasion_warnings) },
};

const config_table invasion_warning_table = {
    "invasion_warnings.json",
    "warning",
    0,
    INVASION_WARNINGS,
    CONFIG_TABLE_COLUMNS(invasion_warning_fields, invasion_warning_storage),
};
