/**
 * @file can_bus.c
 * @brief CAN Bus (TWAI) abstraction layer implementation
 *
 * Provides:
 * - TWAI driver init at 500 kbps
 * - FreeRTOS RX task on Core 0
 * - Callback-based message routing by CAN ID
 * - Bus-off detection and automatic recovery
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

/* ── CAN bus health monitoring ─────────────────────────────────────── */

#define CAN_HEALTH_CHECK_MS     1000   /* Check bus state every 1s */
#define CAN_RECOVERY_TIMEOUT_MS 500    /* Max time to wait for recovery */

static void can_check_bus_health(void)
{
    twai_status_info_t status;
    esp_err_t ret = twai_get_status_info(&status);
    if (ret != ESP_OK) return;

    switch (status.state) {
        case TWAI_STATE_BUS_OFF:
            ESP_LOGW(TAG, "CAN bus-off detected (TX err: %lu, RX err: %lu). Initiating recovery...",
                     status.tx_error_counter, status.rx_error_counter);
            ret = twai_initiate_recovery();
            if (ret == ESP_OK) {
                /* Wait for recovery to complete */
                vTaskDelay(pdMS_TO_TICKS(CAN_RECOVERY_TIMEOUT_MS));
                ret = twai_start();
                if (ret == ESP_OK) {
                    ESP_LOGI(TAG, "CAN bus recovered successfully");
                } else {
                    ESP_LOGE(TAG, "CAN bus restart failed: %s", esp_err_to_name(ret));
                }
            } else {
                ESP_LOGE(TAG, "CAN recovery initiation failed: %s", esp_err_to_name(ret));
            }
            break;

        case TWAI_STATE_RECOVERING:
            ESP_LOGD(TAG, "CAN bus recovering...");
            break;

        case TWAI_STATE_STOPPED:
            ESP_LOGW(TAG, "CAN bus stopped, attempting restart");
            twai_start();
            break;

        default:
            /* RUNNING state - all good */
            break;
    }

    if (status.tx_error_counter > 96 || status.rx_error_counter > 96) {
        ESP_LOGW(TAG, "CAN error counters elevated (TX: %lu, RX: %lu)",
                 status.tx_error_counter, status.rx_error_counter);
    }
}

/* ── CAN RX task ───────────────────────────────────────────────────── */

#define CAN_RX_TASK_STACK  4096
#define CAN_RX_TASK_PRIO   5
#define CAN_RX_TASK_CORE   0

static void can_rx_task(void *arg)
{
    (void)arg;
    twai_message_t msg;
    TickType_t last_health_check = xTaskGetTickCount();

    ESP_LOGI(TAG, "CAN RX task started on core %d", xPortGetCoreID());

    while (1) {
        /* Receive with timeout */
        esp_err_t ret = twai_receive(&msg, pdMS_TO_TICKS(100));

        if (ret == ESP_OK) {
            /* Dispatch to registered handlers */
            for (int i = 0; i < s_handler_count; i++) {
                if (s_handlers[i].msg_id == msg.identifier) {
                    s_handlers[i].handler(msg.identifier, msg.data, msg.data_length_code);
                }
            }
        } else if (ret != ESP_ERR_TIMEOUT) {
            /* Log non-timeout errors */
            ESP_LOGD(TAG, "TWAI receive error: %s", esp_err_to_name(ret));
        }

        /* Periodic bus health check */
        if ((xTaskGetTickCount() - last_health_check) >= pdMS_TO_TICKS(CAN_HEALTH_CHECK_MS)) {
            can_check_bus_health();
            last_health_check = xTaskGetTickCount();
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
        .ss = 1,
    };

    if (data && len > 0) {
        memcpy(msg.data, data, len > 8 ? 8 : len);
    }

    return twai_transmit(&msg, pdMS_TO_TICKS(100));
}
