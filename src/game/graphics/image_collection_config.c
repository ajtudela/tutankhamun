// Config table sg_group_ends.json: per SG file slot the group its file must end at; a load that ends elsewhere
// is logged.

#include "core/config.h"
#include "graphics/image_collection.h"

static const config_field sg_group_end_fields[] = {
    { "group_end", image_sg_expected_group_ends, 1, 1, 0, IMAGE_GROUPS, 0 },
};

static const config_storage sg_group_end_storage[] = {
    { image_sg_expected_group_ends, sizeof(image_sg_expected_group_ends) },
};

const config_table sg_group_end_table = {
    "sg_group_ends.json",
    "slot",
    0,
    IMAGE_SG_SLOTS,
    CONFIG_TABLE_COLUMNS(sg_group_end_fields, sg_group_end_storage),
};
