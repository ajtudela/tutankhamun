// The functions of empire_editor_objects.c, the empire editor's modes and the image kinds of a new object.

#ifndef EDITOR_EMPIRE_EDITOR_OBJECTS_H
#define EDITOR_EMPIRE_EDITOR_OBJECTS_H

#include <stdint.h>

enum empire_editor_mode {
    EMPIRE_EDITOR_MODE_NONE = 0,
    EMPIRE_EDITOR_MODE_ADD_OBJECT = 1,
    EMPIRE_EDITOR_MODE_EDIT_OBJECTS = 2,
    EMPIRE_EDITOR_MODE_DELETE_OBJECT = 3,
    EMPIRE_EDITOR_MODE_GENERAL = 4,
    EMPIRE_EDITOR_MODE_OK = 8,
    EMPIRE_EDITOR_MODE_UNSET_9 = 9,
    EMPIRE_EDITOR_MODE_UNSET_10 = 10,
    EMPIRE_EDITOR_MODE_UNSET_11 = 11,
    EMPIRE_EDITOR_MODE_UNSET_12 = 12,
    EMPIRE_EDITOR_MODE_UNSET_13 = 13,
    EMPIRE_EDITOR_MODE_ADD_ROUTE = 14,
    EMPIRE_EDITOR_MODE_EDIT_ROUTE = 15,
};

enum empire_editor_image_kind {
    EMPIRE_EDITOR_IMAGE_SIMPLE_GRAPHIC = 0,
    EMPIRE_EDITOR_IMAGE_CITY = 1,
    EMPIRE_EDITOR_IMAGE_REGION = 2,
    EMPIRE_EDITOR_IMAGE_BATTLE_MARKER = 3,
};

void empire_editor_city_route_changed(void);
void empire_editor_object_step(void);
void empire_editor_count_objects(void);
void empire_editor_check_city(int32_t index);
void empire_editor_objects_start(void);
void empire_editor_delete_object(int32_t index);
void empire_editor_button(void);
void empire_editor_open_trade_prices(void);
void empire_editor_file_button(void);

#endif
