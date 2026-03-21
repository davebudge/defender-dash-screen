/**
 * @file can_bus.h
 * @brief CAN Bus (TWAI) abstraction layer
 *
 * Provides initialization, message reception via a FreeRTOS task,
 * and callback-based message routing by CAN ID.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of CAN message handlers that can be registered */
#define CAN_MAX_HANDLERS 32

/**
 * @brief Callback type for CAN message handlers
 *
 * @param msg_id  CAN message identifier
 * @param data    Pointer to message data bytes
 * @param len     Number of data bytes (0-8)
 */
typedef void (*can_msg_handler_t)(uint32_t msg_id, const uint8_t *data, uint8_t len);

/**
 * @brief Initialize the TWAI (CAN) driver
 *
 * Configures TWAI at 500 kbps with GPIO pins from Kconfig.
 * Does NOT start the receive task - call can_bus_start_task() for that.
 *
 * @return ESP_OK on success
 */
esp_err_t can_bus_init(void);

/**
 * @brief Start the CAN receive task
 *
 * Spawns a FreeRTOS task on Core 0 that continuously receives CAN
 * messages and dispatches them to registered handlers.
 *
 * @return ESP_OK on success
 */
esp_err_t can_bus_start_task(void);

/**
 * @brief Register a handler for a specific CAN message ID
 *
 * When a message with the given ID is received, the handler will be
 * called from the CAN RX task context. Multiple handlers for the same
 * ID are supported (up to CAN_MAX_HANDLERS total).
 *
 * @param msg_id   CAN message identifier to match
 * @param handler  Callback function
 * @return ESP_OK on success, ESP_ERR_NO_MEM if handler table is full
 */
esp_err_t can_bus_register_handler(uint32_t msg_id, can_msg_handler_t handler);

/**
 * @brief Transmit a CAN message
 *
 * @param msg_id  CAN message identifier
 * @param data    Pointer to data bytes
 * @param len     Number of data bytes (0-8)
 * @return ESP_OK on success
 */
esp_err_t can_bus_transmit(uint32_t msg_id, const uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif
