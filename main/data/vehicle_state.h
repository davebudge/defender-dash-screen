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

/* ── Vehicle operating state (key barrel) ──────────────────────────── */

typedef enum {
    VEHICLE_STATE_OFF = 0,    /**< Key out / screen off */
    VEHICLE_STATE_ACC,        /**< Accessory — screen on, self-test runs */
    VEHICLE_STATE_ON,         /**< Run/Ready — full operation, CAN icons active */
    VEHICLE_STATE_CHARGE,     /**< Charge cable connected */
} vehicle_op_state_t;

typedef enum {
    SELF_TEST_IDLE = 0,       /**< No self-test in progress */
    SELF_TEST_ACTIVE,         /**< ADR tell-tales illuminated (3s) */
    SELF_TEST_COMPLETE,       /**< Self-test finished, normal operation */
} self_test_phase_t;

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

/* ── Vehicle status telemetry ──────────────────────────────────────── */

typedef struct {
    float    soc;                  /**< State of charge (%) */
    float    battery_voltage;      /**< Battery pack voltage (V) */
    float    battery_current;      /**< Battery current (A) */
    float    battery_temp_front;   /**< Battery temperature front (C) */
    float    battery_temp_rear;    /**< Battery temperature rear (C) */
    float    pack_voltage_delta;   /**< Pack voltage delta (V) */
    char     cell_balance[16];     /**< Cell balance status string */
    uint16_t charge_cycles;        /**< Total charge cycles */
    uint8_t  battery_health;       /**< Battery health (%) */
    float    max_discharge_current;/**< Max discharge current (A) */
    uint16_t estimated_range_km;   /**< Estimated remaining range (km) */
    char     last_charged[16];     /**< Last charged date (YYYY-MM-DD) */
    char     next_maintenance[16]; /**< Next maintenance date (YYYY-MM-DD) */
    char     firmware_version[16]; /**< Firmware version string */
} vehicle_status_t;

/**
 * @brief Get a copy of the current vehicle status
 *
 * Thread-safe. Returns a snapshot of all telemetry values.
 */
vehicle_status_t vehicle_state_get_status(void);

/**
 * @brief Update a vehicle status field
 *
 * Thread-safe. Typically called from CAN handler context.
 * Pass a pointer to a fully populated vehicle_status_t.
 */
void vehicle_state_set_status(const vehicle_status_t *status);

/* ── Vehicle operating state API ──────────────────────────────────── */

/**
 * @brief Get the current vehicle operating state
 */
vehicle_op_state_t vehicle_state_get_op_state(void);

/**
 * @brief Set the vehicle operating state (triggers state transitions)
 *
 * OFF→ACC triggers ADR self-test (3s tell-tale illumination).
 * ON and CHARGE cancel any active self-test; only CAN-driven icons shown.
 * OFF clears all icons. Thread-safe.
 */
void vehicle_state_set_op_state(vehicle_op_state_t new_state);

/**
 * @brief Get the current self-test phase
 */
self_test_phase_t vehicle_state_get_self_test_phase(void);

/**
 * @brief Advance self-test timer (call from LVGL timer, 100ms interval)
 *
 * Non-blocking. After 3s, clears self-test icons that have no CAN fault.
 */
void vehicle_state_self_test_tick(uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif
