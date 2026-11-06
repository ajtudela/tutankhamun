// The gameplay config tables of the flotsam (core/config.h): flotsam_frames.json (the bob, debris and sinking frame
// runs), flotsam_starts.json (the kind and first wait of each flotsam) and flotsam_restart_waits.json (the wait before
// a flotsam floats again).

#include "core/config.h"
#include "figure/natives.h"

typedef struct flotsam_frame_run {
    int32_t *frames;
    int32_t count;
} flotsam_frame_run;

static const flotsam_frame_run flotsam_frame_runs[] = {
    { flotsam_bob_frames, CONFIG_COUNT(flotsam_bob_frames) },
    { flotsam_debris_frames, CONFIG_COUNT(flotsam_debris_frames) },
    { flotsam_sinking_frames, CONFIG_COUNT(flotsam_sinking_frames) },
};

static const config_name flotsam_frame_run_names[] = { { "bob", 0 }, { "debris", 1 }, { "sinking", 2 }, { 0, 0 } };

static void *flotsam_frame_block(int32_t record, int32_t *count)
{
    *count = flotsam_frame_runs[record].count;
    return flotsam_frame_runs[record].frames;
}

static const config_field flotsam_frame_element_fields[] = {
    { "offset", 0, 0, 1, FLOTSAM_SUNK_FRAME, 100, 0, 0 },
};

static const config_list flotsam_frame_list = {
    "frame",
    sizeof(int32_t),
    flotsam_frame_element_fields,
    CONFIG_COUNT(flotsam_frame_element_fields),
    flotsam_frame_block,
    0,
};

static const config_field flotsam_frame_fields[] = {
    { "frames", 0, 0, 1, 0, 0, 0, 0, 0, &flotsam_frame_list },
};

const config_table flotsam_frame_table = {
    "flotsam_frames.json",
    "run",
    flotsam_frame_run_names,
    CONFIG_COUNT(flotsam_frame_runs),
    flotsam_frame_fields,
    CONFIG_COUNT(flotsam_frame_fields),
    0,
    0,
};

static const config_field flotsam_start_fields[] = {
    { "kind", &flotsam_starts[0].kind, 2, 1, FLOTSAM_KIND_BOBBING, FLOTSAM_KIND_SINKING, config_flotsam_kind_names },
    { "wait", &flotsam_starts[0].wait, 2, 1, 0, 32767, 0 },
};

static const config_storage flotsam_start_storage[] = {
    { flotsam_starts, sizeof(flotsam_starts) },
};

const config_table flotsam_start_table = {
    "flotsam_starts.json",
    "flotsam",
    0,
    FLOTSAM_COUNT,
    CONFIG_TABLE_COLUMNS(flotsam_start_fields, flotsam_start_storage),
};

static const config_field flotsam_restart_wait_fields[] = {
    { "floated_at_least", &flotsam_restart_waits[0].floated_at_least, 3, 1, 0, 32767, 0 },
    { "random_mask", &flotsam_restart_waits[0].random_mask, 3, 1, 0, 32767, 0 },
    { "base", &flotsam_restart_waits[0].base, 3, 1, 0, 32767, 0 },
};

static const config_storage flotsam_restart_wait_storage[] = {
    { flotsam_restart_waits, sizeof(flotsam_restart_waits) },
};

const config_table flotsam_restart_wait_table = {
    "flotsam_restart_waits.json",
    "step",
    0,
    FLOTSAM_RESTART_STEPS,
    CONFIG_TABLE_COLUMNS(flotsam_restart_wait_fields, flotsam_restart_wait_storage),
};
