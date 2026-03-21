/**
 * @file vehicle_state.h
 * @brief Central vehicle state - bridges CAN bus data to UI
 *
 * This module holds the current state of every dashboard indicator.
 * It provides thread-safe access (FreeRTOS mutex) since CAN updates
 * come from Core 0 and UI reads happen on Core 1.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "icon_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Icon states ───────────────────────────────────────────────────── */

typedef enum {
    ICON_STATE_OFF = 0,
    ICON_STATE_ON,
    ICON_STATE_BLINK,
} icon_state_t;

/* ── Gear positions ────────────────────────────────────────────────── */

typedef enum {
    GEAR_P = 0,
    GEAR_R,
    GEAR_N,
    GEAR_D,
    GEAR_S,
    GEAR_COUNT
} gear_t;

/**
 * @brief Initialize the vehicle state module
 *
 * Creates the mutex and registers CAN message handlers for all
 * icons that have a non-zero can_msg_id in icon_defs[].
 */
void vehicle_state_init(void);

/**
 * @brief Get the current state of a warning icon
 *
 * Thread-safe. Can be called from any core.
 *
 * @param id  Icon identifier
 * @return Current icon state (OFF, ON, or BLINK)
 */
icon_state_t vehicle_state_get_icon(icon_id_t id);

/**
 * @brief Set the state of a warning icon
 *
 * Thread-safe. Typically called from CAN handler context.
 *
 * @param id     Icon identifier
 * @param state  New icon state
 */
void vehicle_state_set_icon(icon_id_t id, icon_state_t state);

/**
 * @brief Get the current gear position
 *
 * Thread-safe.
 *
 * @return Current gear (P, R, N, D, or S)
 */
gear_t vehicle_state_get_gear(void);

/**
 * @brief Set the current gear position
 *
 * Thread-safe. Typically called from CAN handler context.
 *
 * @param gear  New gear position
 */
void vehicle_state_set_gear(gear_t gear);

/**
 * @brief Check if the state has changed since last UI update
 *
 * @return true if any state changed since last call to this function
 */
bool vehicle_state_is_dirty(void);

#ifdef __cplusplus
}
#endif
