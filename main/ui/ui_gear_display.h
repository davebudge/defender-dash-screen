/**
 * @file ui_gear_display.h
 * @brief Central gear selector display (P/R/N/D/S)
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the gear display widget
 *
 * Creates a large centered label showing the current gear letter.
 * Uses Eurostile Extended Bold font if available, falls back to Montserrat 48.
 *
 * @param parent  Container for the gear display
 */
void ui_gear_display_create(lv_obj_t *parent);

/**
 * @brief Update the gear display from vehicle_state
 *
 * Must be called from LVGL task context.
 */
void ui_gear_display_update(void);

#ifdef __cplusplus
}
#endif
