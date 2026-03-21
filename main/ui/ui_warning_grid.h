/**
 * @file ui_warning_grid.h
 * @brief Data-driven warning light icon grid
 *
 * Creates and manages lv_img objects for each icon defined in icon_defs[].
 * Icons are positioned in a grid layout matching the Figma design.
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the warning light grid
 *
 * Reads icon_defs[] table and creates lv_img objects for each icon.
 * Icons in rows 0-2 are placed in top_parent (above gear display).
 * Icons in rows 3+ are placed in bot_parent (below gear display).
 *
 * @param top_parent  Container for icons above the gear display
 * @param bot_parent  Container for icons below the gear display
 */
void ui_warning_grid_create(lv_obj_t *top_parent, lv_obj_t *bot_parent);

/**
 * @brief Update all icon states from vehicle_state
 *
 * Must be called from LVGL task context (e.g., from an lv_timer).
 * Reads current icon states and updates opacity/color accordingly.
 */
void ui_warning_grid_update(void);

#ifdef __cplusplus
}
#endif
