/**
 * @file can_bus.c
 * @brief CAN Bus (TWAI) abstraction layer implementation
 */

#include "can_bus.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"
#include "esp_log.h"

static const char *TAG = "CAN";

/* ── Handler registry ──────────────────────────────────────────────── */

typedef struct {
    uint32_t          msg_id;
    can_msg_handler_t handler;
} can_handler_entry_t;

static can_handler_entry_t s_handlers[CAN_MAX_HANDLERS];
static int s_handler_count = 0;

/* ── CAN RX task ───────────────────────────────────────────────────── */

#define CAN_RX_TASK_STACK  4096
#define CAN_RX_TASK_PRIO   5
#define CAN_RX_TASK_CORE   0

static void can_rx_task(void *arg)
{
    (void)arg;
    twai_message_t msg;

    ESP_LOGI(TAG, "CAN RX task started on core %d", xPortGetCoreID());

    while (1) {
        /* Block for up to 100ms waiting for a message */
        esp_err_t ret = twai_receive(&msg, pdMS_TO_TICKS(100));
        if (ret != ESP_OK) {
            continue;  /* Timeout or error - just try again */
        }

        /* Dispatch to registered handlers */
        for (int i = 0; i < s_handler_count; i++) {
            if (s_handlers[i].msg_id == msg.identifier) {
                s_handlers[i].handler(msg.identifier, msg.data, msg.data_length_code);
            }
        }
    }
}

/* ── Public API ────────────────────────────────────────────────────── */

esp_err_t can_bus_init(void)
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        CONFIG_EXAMPLE_TX_GPIO_NUM,
        CONFIG_EXAMPLE_RX_GPIO_NUM,
        TWAI_MODE_NORMAL
    );
    g_config.rx_queue_len = 32;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "TWAI driver started (500 kbps, TX=%d, RX=%d)",
             CONFIG_EXAMPLE_TX_GPIO_NUM, CONFIG_EXAMPLE_RX_GPIO_NUM);
    return ESP_OK;
}

esp_err_t can_bus_start_task(void)
{
    BaseType_t ret = xTaskCreatePinnedToCore(
        can_rx_task,
        "can_rx",
        CAN_RX_TASK_STACK,
        NULL,
        CAN_RX_TASK_PRIO,
        NULL,
        CAN_RX_TASK_CORE
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create CAN RX task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t can_bus_register_handler(uint32_t msg_id, can_msg_handler_t handler)
{
    if (s_handler_count >= CAN_MAX_HANDLERS) {
        ESP_LOGE(TAG, "Handler table full (max %d)", CAN_MAX_HANDLERS);
        return ESP_ERR_NO_MEM;
    }

    s_handlers[s_handler_count].msg_id = msg_id;
    s_handlers[s_handler_count].handler = handler;
    s_handler_count++;

    ESP_LOGI(TAG, "Registered handler for CAN ID 0x%03lX (total: %d)",
             (unsigned long)msg_id, s_handler_count);
    return ESP_OK;
}

esp_err_t can_bus_transmit(uint32_t msg_id, const uint8_t *data, uint8_t len)
{
    twai_message_t msg = {
        .identifier = msg_id,
        .data_length_code = len,
        .ss = 1,  /* Single-shot mode */
    };

    if (data && len > 0) {
        memcpy(msg.data, data, len > 8 ? 8 : len);
    }

    return twai_transmit(&msg, pdMS_TO_TICKS(100));
}
