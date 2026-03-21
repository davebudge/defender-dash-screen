/**
 * @file ui_main.h
 * @brief Top-level UI initialization and layout
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the dashboard UI
 *
 * Creates the root screen with black background, positions the
 * warning grid containers and gear display, and starts the 100ms
 * update timer.
 *
 * Must be called within LVGL lock context (lvgl_port_lock).
 */
void ui_init(void);

#ifdef __cplusplus
}
#endif
