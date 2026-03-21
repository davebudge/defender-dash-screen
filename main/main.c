/**
 * @file main.c
 * @brief Defender Dashboard - Application Entry Point
 *
 * Initializes hardware (LCD, GPIO expander, CAN bus) and launches the
 * dashboard UI. The LVGL task runs on Core 1, CAN RX task on Core 0.
 */

#include "waveshare_rgb_lcd_port.h"
#include "lvgl_port.h"
#include "CH422G.h"
#include "DEV_Config.h"
#include "can_bus.h"
#include "ui_main.h"
#include "vehicle_state.h"

void app_main(void)
{
    // 1. Initialize display hardware (RGB LCD + touch + backlight)
    waveshare_esp32_s3_rgb_lcd_init();

    // 2. Initialize I2C master (used by CH422G GPIO expander)
    DEV_Module_Init();

    // 3. Initialize vehicle state (must be before CAN so handlers are ready)
    vehicle_state_init();

    // 4. Initialize CAN bus (TWAI driver)
    ESP_ERROR_CHECK(can_bus_init());

    // 5. Initialize LVGL UI from within LVGL context
    if (lvgl_port_lock(-1)) {
        ui_init();
        lvgl_port_unlock();
    }

    // 6. Start CAN bus receive task (runs on Core 0)
    ESP_ERROR_CHECK(can_bus_start_task());
}
