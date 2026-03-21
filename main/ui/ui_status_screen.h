/**
 * @file ui_status_screen.h
 * @brief Vehicle status telemetry screen
 *
 * Displays battery and vehicle data in a scrollable list.
 */

#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create the vehicle status screen
 *
 * @param parent  Container for the status screen content
 */
void ui_status_screen_create(lv_obj_t *parent);

/**
 * @brief Update all status values from vehicle_state
 *
 * Must be called from LVGL task context.
 */
void ui_status_screen_update(void);

#ifdef __cplusplus
}
#endif
