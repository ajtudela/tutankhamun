// The city drawn at the current camera, for the windows that show the city and the sidebar slide. Drawing
// also advances the saved tile animation state, so runs without a window draw it too.

#include "widget/city_draw_view.h"
#include "building/building_data.h"
#include "widget/city_draw.h"
#include <stdint.h>

void widget_city_draw_current_view(void)
{
    widget_city_draw(map_info.camera_x, map_info.camera_y, map_info.grid_start);
}
