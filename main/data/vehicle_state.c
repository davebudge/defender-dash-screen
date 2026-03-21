/**
 * @file vehicle_state.c
 * @brief Central vehicle state implementation
 */

#include "vehicle_state.h"
#include "can_bus.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "VSTATE";

/* ── Internal state ────────────────────────────────────────────────── */

static SemaphoreHandle_t s_mutex = NULL;
static icon_state_t s_icon_states[ICON_COUNT] = {0};  /* All OFF initially */
static gear_t s_gear = GEAR_P;
static bool s_dirty = true;  /* Start dirty so first UI update runs */
static vehicle_status_t s_status = {
    .soc = 0.0f,
    .battery_voltage = 0.0f,
    .battery_current = 0.0f,
    .battery_temp_front = 0.0f,
    .battery_temp_rear = 0.0f,
    .pack_voltage_delta = 0.0f,
    .cell_balance = "---",
    .charge_cycles = 0,
    .battery_health = 0,
    .max_discharge_current = 0.0f,
    .estimated_range_km = 0,
    .last_charged = "---",
    .next_maintenance = "---",
    .firmware_version = "---",
};

/* ── CAN message handlers ──────────────────────────────────────────── */

/**
 * @brief Generic CAN handler for icon state messages
 *
 * Looks up the message ID in icon_defs[] and sets icon state
 * based on the configured byte and bit mask.
 */
static void on_can_icon_msg(uint32_t msg_id, const uint8_t *data, uint8_t len)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);

    for (int i = 0; i < ICON_COUNT; i++) {
        if (icon_defs[i].can_msg_id == msg_id) {
            uint8_t byte_idx = icon_defs[i].can_byte;
            uint8_t mask = icon_defs[i].can_bit_mask;

            if (byte_idx < len) {
                bool active = (data[byte_idx] & mask) != 0;
                icon_state_t new_state = active ? ICON_STATE_ON : ICON_STATE_OFF;

                if (s_icon_states[i] != new_state) {
                    s_icon_states[i] = new_state;
                    s_dirty = true;
                    ESP_LOGD(TAG, "Icon %s -> %s",
                             icon_defs[i].name,
                             active ? "ON" : "OFF");
                }
            }
        }
    }

    xSemaphoreGive(s_mutex);
}

/* ── Public API ────────────────────────────────────────────────────── */

void vehicle_state_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) {
        ESP_LOGE(TAG, "FATAL: Failed to create vehicle_state mutex");
        abort();
    }

    /* Auto-register CAN handlers for all icons with mapped CAN IDs.
     * We track which IDs we've already registered to avoid duplicates
     * (multiple icons can share the same CAN message). */
    uint32_t registered_ids[CAN_MAX_HANDLERS];
    int registered_count = 0;

    for (int i = 0; i < ICON_COUNT; i++) {
        uint32_t mid = icon_defs[i].can_msg_id;
        if (mid == 0) continue;

        /* Check if we already registered this message ID */
        bool already_registered = false;
        for (int j = 0; j < registered_count; j++) {
            if (registered_ids[j] == mid) {
                already_registered = true;
                break;
            }
        }

        if (!already_registered) {
            can_bus_register_handler(mid, on_can_icon_msg);
            registered_ids[registered_count++] = mid;
        }
    }

    ESP_LOGI(TAG, "Vehicle state initialized (%d CAN handlers registered)", registered_count);
}

icon_state_t vehicle_state_get_icon(icon_id_t id)
{
    icon_state_t state = ICON_STATE_OFF;
    if (id < ICON_COUNT) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        state = s_icon_states[id];
        xSemaphoreGive(s_mutex);
    }
    return state;
}

void vehicle_state_set_icon(icon_id_t id, icon_state_t state)
{
    if (id < ICON_COUNT) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        if (s_icon_states[id] != state) {
            s_icon_states[id] = state;
            s_dirty = true;
        }
        xSemaphoreGive(s_mutex);
    }
}

gear_t vehicle_state_get_gear(void)
{
    gear_t gear;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    gear = s_gear;
    xSemaphoreGive(s_mutex);
    return gear;
}

void vehicle_state_set_gear(gear_t gear)
{
    if (gear < GEAR_COUNT) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        if (s_gear != gear) {
            s_gear = gear;
            s_dirty = true;
            ESP_LOGI(TAG, "Gear -> %c", "PRNDS"[gear]);
        }
        xSemaphoreGive(s_mutex);
    }
}

bool vehicle_state_is_dirty(void)
{
    bool dirty;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    dirty = s_dirty;
    s_dirty = false;
    xSemaphoreGive(s_mutex);
    return dirty;
}

vehicle_status_t vehicle_state_get_status(void)
{
    vehicle_status_t status;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    status = s_status;
    xSemaphoreGive(s_mutex);
    return status;
}

void vehicle_state_set_status(const vehicle_status_t *status)
{
    if (status == NULL) return;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_status = *status;
    s_dirty = true;
    xSemaphoreGive(s_mutex);
}
